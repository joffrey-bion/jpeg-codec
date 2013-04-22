#include "../include/analyse_header.h"

/* 
 * Procedure annexe utilitaire :
 * retrouve l'indice i d'une composante dans le tableau a partir de son ic
 */
uint32_t indice_comp(uint8_t ic, image_t *im)
{
	for (uint32_t i = 0; i < im->nb_comp; i++) {
		if (im->comp[i].ic == ic) {
			return i;
		}
	}
	return 0xFFFFFFFF;
}

void unsupported_mode(char *s)
{
	printf("\n** ERREUR: mode %s non supporté par ce décodeur **\n",s);
}

/* PROCEDURES ANNEXES D'ANALYSE DES BLOCS */

/* Analyse un bloc DHT - recupere les tables de Huffman */
void read_DHT(huff_table_t *HT[2][2], uint32_t *cpt, FILE *f)
{
	uint32_t end = *cpt + read_block_size(cpt,f);
	while (end != *cpt) {
		printhead("%5u :  ",*cpt);
		uint8_t info = read_byte(cpt,f);
		if ((info >> 5) != 0) {
			printf("\n** ERREUR: type AC/DC Hufftable errone **\n");
			return;
		}
		uint32_t indice = info % 16;
		assert(indice == 0 || indice == 1);
		uint32_t type = info >> 4; /* 0 -> DC   1 -> AC */
		assert(type == 0 || type == 1);
		if (type == 0) {
			printhead("    ind = %u   /   type = DC\n",indice);
		} else {
			printhead("    ind = %u   /   type = AC\n",indice);
		}
		assert(HT[type][indice] == NULL);
		HT[type][indice] = malloc(sizeof(huff_table_t));
		assert(HT[type][indice] != NULL);
		load_huffman_table(cpt,f,HT[type][indice]);
	}
	assert(end == *cpt);
}


/* Analyse un bloc DQT - recupere les tables de quantification */
void read_DQT(uint8_t QT[2][_BSIZE], uint32_t *cpt, FILE *f)
{
	uint32_t end = *cpt + read_block_size(cpt,f);
	while (end != *cpt) {
		printhead("%5u :  ",*cpt);
		uint8_t prec_iq = read_byte(cpt,f);
		uint32_t prec = 8 + 8 * (prec_iq >> 4); /* 0 = 8bit  1 = 16bit */
		uint32_t iq = prec_iq % 16;
		printhead("    precision = %ubit     iq = %u\n",prec,iq);
		if (prec == 16) {
			printf("\n** ERREUR: Precision 16bit non supportee\n");
			return;
		}
		printhead("%5u :  ",*cpt);
		printhead("    table : ┌─────────────────────────┐\n");
		for (uint32_t i = 0; i < _BHEIGHT; i++) {
			printhead("%5u :              │ ",*cpt);
			for (uint32_t j = 0; j < _BWIDTH; j++) {
				uint8_t oct = read_byte(cpt,f);
				printhead("%2x ",oct);
				QT[iq][i * _BWIDTH + j] = oct;
			}
			printhead("│\n");
		}
		printhead("                     └─────────────────────────┘\n");
	}
	assert(end == *cpt);
}


/* Analyse le bloc SOF0 - recupere les proprietes de l'image */
void read_SOF0(image_t *im, uint32_t *cpt, FILE *f)
{
        uint32_t end = *cpt + read_block_size(cpt,f);
	im->precision = read_byte(cpt,f);
	printhead("%5u :      precision = %ubit\n",*cpt-1,im->precision);
	im->height = read_2bytes(cpt,f);
	printhead("%5u :      hauteur = %upx\n",*cpt-2,im->height);
	im->width = read_2bytes(cpt,f);
	printhead("%5u :      largeur = %upx\n",*cpt-2,im->width);
	im->nb_comp = read_byte(cpt,f);
	printhead("%5u :      nb comp. = %u\n",*cpt-1,im->nb_comp);
	/* creation du tableau de description des composantes */
	im->comp = calloc(im->nb_comp,sizeof(comp_t));
	for (uint32_t i = 0; i < im->nb_comp; i++) {
		im->comp[i].ic = read_byte(cpt,f);
		printhead("%5u :      composante %u :\n",*cpt-1,im->comp[i].ic);
		uint8_t ech = read_byte(cpt,f);
		im->comp[i].echH = ech >> 4;
		im->comp[i].echV = ech % 16;
		printhead("%5u :        ├ ech.H = %u  ech.V = %u\n",*cpt-1,
			  im->comp[i].echH, im->comp[i].echV);
		im->comp[i].iq = read_byte(cpt,f);
		printhead("%5u :        └ n°table Q = %u\n",*cpt-1,
			  im->comp[i].iq);	
	}
	assert(end == *cpt);
}


/* Analyse un bloc DRI - recupere l'intervalle de resynchronisation */
void read_DRI(uint16_t *RI, uint32_t *cpt, FILE *f)
{
	uint32_t end = *cpt + read_block_size(cpt,f);
	printhead("%5u :  ",*cpt);
	*RI = read_2bytes(cpt,f);
	printhead("    intervalle de resync = %u MCU\n",*RI);
	assert(end == *cpt);
}


/* Analyse un bloc COM - affiche le commentaire */
void read_COM(uint32_t *cpt, FILE *f)
{
	uint32_t end = *cpt + read_block_size(cpt,f);
	printhead("%5u :      [commentaire :]\n",*cpt);
	printhead("──────────\n");
	while (*cpt != end) {
		uint8_t oct = read_byte(cpt,f);
		printhead("%c",oct);
	}
	printhead("\n──────────\n");
	assert(end == *cpt);
}

/* Analyse un bloc APP0 - verifie l'encapsulage JFIF */
void read_APP0(uint32_t *cpt, FILE *f)
{
	uint16_t size = read_block_size(cpt,f);
	uint32_t end = *cpt + size - 2;
	char jfif[] = "JFIF";
	printhead("%5u :      ",*cpt);
	for (uint8_t i = 0; i < strlen(jfif); i++) {
		uint8_t oct = read_byte(cpt,f);
		assert(oct == jfif[i]);
		printhead("%c",oct);
	}
	printhead("\n");
	skip_size(size-6,cpt,f);
	assert(end == *cpt);
}

/* Analyse le bloc SOS - recupere les dernieres informations */
void read_SOS(image_t *im, uint32_t *cpt, FILE *f)
{
	uint32_t end = *cpt + read_block_size(cpt,f);
	uint32_t N = read_byte(cpt,f);
	printhead("%5u :      Nb composantes : %u\n",*cpt-1,N);
	assert(N == im->nb_comp);
	/* completion de la description des composantes */
	for (uint32_t i = 0; i < N; i++) {
		uint8_t ic = read_byte(cpt,f);
		printhead("%5u :      Composante %u :\n",*cpt-1,ic);
		uint32_t n = indice_comp(ic, im);
		uint8_t ih = read_byte(cpt,f);
		im->comp[n].ih_DC = ih >> 4;
		im->comp[n].ih_AC = ih % 16;
		printhead("%5u :        └ ih_DC = %u   /  ih_AC = %u\n",*cpt-1,
			  im->comp[n].ih_DC, im->comp[n].ih_AC);
	}
	/* lecture de la fin du bloc (informations non utilisees) */
	uint32_t oct = read_byte(cpt,f);
	printhead("%5u :      deb sel : %u\n",*cpt-1,oct);
	oct = read_byte(cpt,f);
	printhead("%5u :      fin sel : %u\n",*cpt-1,oct);
	oct = read_byte(cpt,f);
	printhead("%5u :      Ah = %u   /  Al = %u\n",*cpt-1,oct >> 4,oct % 16);
	assert(end == *cpt);
}


/* ANALYSEUR D'EN-TETE */

/* Parse l'en-tete et stocke les informations dans les tables */
int8_t analyse_header(image_t *im, uint16_t *RI, uint8_t QT[2][_BSIZE], 
		      huff_table_t *HT[2][2],uint32_t *cpt, FILE *f)
{
	while (feof(f) == 0) {
		printmark("%5u :  ",*cpt-1);
		uint8_t oct = 0;
		oct = read_byte(cpt,f);
		if (oct == 0x01) {
			printmark("TEM\n");
			skip_segment(cpt,f);
		} else if (oct == 0xc0) {
			printmark("SOF0 (Start Of Frame Baseline DCT (Huff))\n");
			read_SOF0(im,cpt,f);
			oct = read_byte(cpt,f);
			assert(oct  ==  0xff);
		} else if (oct == 0xc1) {
			printmark("SOF1 (Start Of Frame DCT seq.extended)\n");
			unsupported_mode("SOF1");
			return -1;
		} else if (oct == 0xc2) {
			printmark("SOF2 (Start Of Frame DCT progressive)\n");
			unsupported_mode("SOF2");
			return -1;
		} else if (oct == 0xc3) {
			printmark("SOF3 (Start Of Frame DCT spatial no loss)\n");
			unsupported_mode("SOF3");;
			return -1;
		} else if (oct == 0xc4) {
			printmark("DHT (Define Huffman Tables)\n");
			read_DHT(HT,cpt,f);
			oct = read_byte(cpt,f);
			assert(oct  ==  0xff);
		} else if (oct == 0xc5) {
			printmark("SOF5 (Start Of Frame DCT seq.diff)\n");
			unsupported_mode("SOF5");;
			return -1;
		} else if (oct == 0xc6) {
			printmark("SOF6 (Start Of Frame DCT seq.progressive)\n");
			unsupported_mode("SOF6");;
			return -1;
		} else if (oct == 0xc7) {
			printmark("SOF7 (Start Of Frame DCT diff.spatial)\n");
			unsupported_mode("SOF7");;
			return -1;
		} else if (oct == 0xc8) {
			printmark("JPG (extensions du JPEG)\n");
			skip_segment(cpt,f);
		} else if (oct == 0xd8) {
			printmark("SOI (Start Of Image)\n");
			oct = read_byte(cpt,f);
			assert(oct == 0xff);
		} else if (oct == 0xd9) {
			printmark("EOI (End Of Image)\n");
			return -1;
		} else if (oct == 0xda) {
			printmark("SOS (Start Of Scan)\n");
			read_SOS(im,cpt,f);
			return 0;
		} else if (oct == 0xdb) {
			printmark("DQT (Define Quantization Tables)\n");
			read_DQT(QT,cpt,f);
			oct = read_byte(cpt,f);
			assert(oct == 0xff);
		} else if (oct == 0xdd) {
			printmark("DRI (Define Restart Interval)\n");
			read_DRI(RI,cpt,f);
			oct = read_byte(cpt,f);
			assert(oct == 0xff);
		} else if (oct == 0xfe) {
			printmark("COM (Commentaire)\n");
			read_COM(cpt,f);
			oct = read_byte(cpt,f);
			assert(oct == 0xff);
		} else if (oct == 0xe0) {
			printmark("APP0 (Application Marker)\n");
			read_APP0(cpt,f);
			oct = read_byte(cpt,f);
			assert(oct == 0xff);
		} else if ((oct>>4)%16 == 0xe) {
			printmark("APP%u (Application Marker)\n",oct % 16);
			skip_segment(cpt,f);
		} else if ((oct>>4)%16 == 0xf && oct % 16 != 0xf) {
			printmark("JPG%u (JPEG Marker)\n",oct % 16);
			skip_segment(cpt,f);
		} else {
			printf("marqueur inconnu : 0xff%02x\n",oct);
		}
		printhead("\n");
	}
	printf("** ERREUR: Pas de bloc SOS trouvé **\n");
	return -1;
}
