#include "../include/inout.h"

uint8_t read_byte(uint32_t *cpt, FILE *f)
{
	uint8_t oct = 0;
	size_t check = fread(&oct,1,1,f);
	if (check != 1) {
		if (feof(f)) {
			printf("End Of File\n");
		} else {
			printf("\nErreur: lecture impossible\n");
		}
	}
	(*cpt)++;
	return oct;
}

uint16_t read_2bytes(uint32_t *cpt, FILE *f)
{
	uint16_t oct1 = 0;
	uint16_t oct2 = 0;
	oct1 = read_byte(cpt,f);
	oct2 = read_byte(cpt,f);
	return (oct1 << 8) + oct2;
}

uint32_t read_block_size(uint32_t *cpt, FILE *f)
{
	uint32_t size = read_2bytes(cpt,f);
	printhead("%5u :      taille du segment = %u\n",*cpt-2,size);
	return size;
}


void skip_size(uint32_t size, uint32_t *cpt, FILE *f)
{
        printhead("%5u :      [passe %u octets]\n",*cpt,size);
	uint8_t oct = 0;
	for (uint32_t i = 0; i < size; i++) {
		oct = read_byte(cpt,f);
	}
}

int8_t display_data_byte(uint8_t space, uint8_t first, uint32_t *cpt, FILE *f)
{
	uint8_t curr = read_byte(cpt,f);
	if (curr == 0xff) {
		curr = read_byte(cpt,f);
		if (curr == 0x00) {
			printhead("ff");
		} else if (curr == 0xd9) {
			printhead("\n%5u :  EOI (End Of Image)\n",*cpt-1);
			return -1;
		} else if ((curr >> 4) == 0xd && (curr % 16) < 8) {
			if (first == 0) {
				printhead("\n%5u :  ",*cpt-1);
			}
			printhead("RST%u",(curr % 16));
			curr = read_byte(cpt,f);
			return 1;
		} else {
			printf("\nERREUR: marqueur inattendu\n");
			return -1;
		}
	} else {
		printhead("%02x",curr);
	}
	if (space) {
		printhead(" ");
	}
	return 0;
}

void display_data_and_EOI(uint32_t *cpt, FILE *f)
{
	while (feof(f) == 0) {
		printhead("%5u :  ",*cpt);
		for (uint32_t i = 0; i < 16; i++) {
			int8_t end = display_data_byte(i % 2, i == 0, cpt, f);
			if (end == 1) {
				break;
			} else if (end == -1) {
				return;
			}
		}
		printhead("\n");
	}
	display_data_and_EOI(cpt,f);
}

void print_freq_block(int32_t T[_BSIZE])
{
	for (uint32_t i = 0; i < _BHEIGHT; i++) {
		for (uint32_t j = 0; j < _BWIDTH; j++) {
			printf(" %i ",T[i*_BWIDTH + j]);
		}
		printf("\n");
	}
}

void print_px_MCU(uint32_t *MCU, uint8_t MCU_size[2])
{
	for (uint32_t i = 0; i < MCU_size[1]; i++) {
		for (uint32_t j = 0; j < MCU_size[0]; j++) {
			printf(" %06x ",MCU[i*MCU_size[0] + j]);
		}
		printf("\n");
	}
}

