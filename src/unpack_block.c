#include "../include/unpack_block.h"

uint8_t read_bit(FILE *f, scan_desc_t *scan_desc)
{
	uint32_t cpt = 0;
	if (scan_desc->bit_count == 0) {
		uint32_t pred = scan_desc->window;
		scan_desc->window = read_byte(&cpt,f);
		if (pred == 0xff) {
			assert(scan_desc->window == 0x00);
			scan_desc->window = read_byte(&cpt,f);
		}
	}
	uint8_t byte = scan_desc->window;
	uint8_t numbit = scan_desc->bit_count;
	byte = byte >> (7 - numbit);
	uint8_t bit = byte & 1;
	scan_desc->bit_count += 1;
	if (scan_desc->bit_count == 8) {
		scan_desc->bit_count = 0;
	}
	assert(bit == 0 || bit == 1);
	return bit;
}

/*
 * decode une suite de bits du fichier (longueur variable) a l'aide de la 
 * table de huffman et retourne le symbole associe
 */
uint8_t read_symbol(FILE *f, scan_desc_t *scan_desc, huff_table_t *table)
{
	assert(table != NULL);
	uint8_t bit;
	uint16_t code = 0;
	while (table->is_elt == 0) {
		bit = read_bit(f, scan_desc);
		if (bit == 0) {
			code = 2*code;
			table = table->left;
		} else {
			code = 2*code + 1;
			table = table->right;
		}
		if (table == NULL) {
			printf("\n** ERREUR: code inconnu rencontré : %u **\n",code);
			return -1;
		}
	}
	assert(table->is_elt);
	printdecod("%u(%u) ",table->value,table->code);
	return table->value;
}

/*
 * lit une suite de bits du fichier (de longueur 'magnitude') et 
 * retourne la valeur correspondante pour cette classe de magnitude
 */
uint32_t read_value(FILE *f, scan_desc_t *scan_desc, uint8_t magnitude)
{
	int32_t value = 0;
	for (uint32_t i = 0; i < magnitude; i++) {
		value = 2 * value + read_bit(f,scan_desc);		
	}
	int32_t max = (1 << magnitude) - 1;
	if (value <= max/2) {
		value = -(max-value);
	}
	assert(value <= max);
	assert(value >= (-max));
	return value;	
}


uint32_t calls_count = 1;

void unpack_block(FILE *f, scan_desc_t *scan_desc, uint32_t index, 
		  int32_t T[_BSIZE])
{
	printdecod("Start unpack_block #%u (index = %u)\n",calls_count,index);
	assert(feof(f) == 0);
	huff_table_t *DC = scan_desc->table[0][index];
	huff_table_t *AC = scan_desc->table[1][index];
	if (DC == NULL || AC == NULL) {
		printf("\n** ERREUR: Table de Huffman absente **\n");
	}
	assert(DC != NULL);
	assert(AC != NULL);
	/* decodage du coefficient DC */
	uint8_t magnitude = read_symbol(f,scan_desc,DC);
	int32_t offset = read_value(f,scan_desc,magnitude);
	T[0] = scan_desc->pred[index] + offset;
	scan_desc->pred[index] = T[0];
	/* decodage des coefficients AC */
	uint8_t symbol;
	uint8_t nb_zero;
	uint32_t i = 1;
	while (i < 64) {
		symbol = read_symbol(f,scan_desc,AC);
		if (symbol == 0xf0) {        /* ZRL */
			for (uint32_t j = 0; j < 16; j++) {
				T[i] = 0;
				i++;
			}	
		} else if (symbol == 0x00) { /* EOB */
			while (i < 64) {
				T[i] = 0;
				i++;
			}
		} else {                     /* symbole normal */
			magnitude = symbol % 16;
			nb_zero = symbol >> 4;
			printdecod("=> sym: %u   mag: %u  nb_zero: %u\n",symbol,
			   magnitude,nb_zero);
			for (uint32_t j = 0; j < nb_zero; j++) {
				T[i] = 0;
				i++;
			}
			T[i] = read_value(f,scan_desc,magnitude);
			i++;
		}
	}
	printdecod("\n");
	printblock(T);
	calls_count++;
}
