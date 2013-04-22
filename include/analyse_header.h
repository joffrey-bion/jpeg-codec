#ifndef __ANALYSE_HEADER_H__
#define __ANALYSE_HEADER_H__

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "jpeg.h"
#include "inout.h"
#include "huffman.h"
#include "skip_segment.h"


/* decrit une composante et ses attributs */
typedef struct _comp_t {
	uint8_t ic;     /* indice de composante */
	uint8_t echV;   /* facteur d'echantillonnage vertical */
	uint8_t echH;   /* facteur d'echantillonnage horizontal */
	uint8_t iq;     /* indice de la table de quantification */
	uint8_t ih_DC;  /* indice de la table de Huffman pour le coeff DC */
	uint8_t ih_AC;  /* indice de la table de Huffman pour les coeff AC */
} comp_t;


/* decrit l'image et ses attributs */
typedef struct _image_t {
	uint8_t precision;  /* precision des valeurs lues (en bit) */
	uint16_t width;     /* largeur de l'image */
	uint16_t height;    /* hauteur de l'image */
	uint8_t nb_comp;    /* nombre de composantes couleurs de l'image */
	comp_t *comp;       /* tableau de description des composantes */
} image_t;


extern int8_t analyse_header(image_t *im, 
			     uint16_t *RI, 
			     uint8_t QT[2][_BSIZE], 
			     huff_table_t *HT[2][2],
			     uint32_t *cpt, 
			     FILE *f);

#endif
