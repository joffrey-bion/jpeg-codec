#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include "../include/inout.h"
#include <string.h>

//#define CACHE_REMPLI(deplacement) cache_cour == fin_cache + (width_image - deplacement)*taille_pix+1

const uint32_t taille_pix = 3;
uint8_t *cache;
uint8_t *cache_cour;
uint8_t *fin_cache;
uint32_t width_image;
uint32_t MCU_h;

/* les procédures d'écriture procèdent en little-endian */
void ecrire_octet(uint8_t oct, FILE *f)
{
	fwrite(&oct,1,1,f);
}

void ecrire_2oct(uint16_t oct2,FILE *f)
{
	uint8_t oct_fort = oct2 & 0x00FF;
	uint8_t oct_faible = oct2 >> 8;
	fwrite(&oct_fort,1,1,f);
	fwrite(&oct_faible,1,1,f);
}

void ecrire_4oct(uint32_t oct4,FILE *f)
{
	uint8_t oct_fort = oct4 & 0xFF;
	uint8_t oct2 = (oct4 >> 8);
	uint8_t oct3 = oct4 >> 16;
        uint8_t oct_faible = oct4 >> 24;

	fwrite(&oct_fort,1,1,f);
	fwrite(&oct2,1,1,f);
	fwrite(&oct3,1,1,f);
	fwrite(&oct_faible,1,1,f);
}

void ecrire_entree(uint16_t tag, uint16_t type, uint32_t nb_val, uint32_t val,
		   FILE *f)
{
	ecrire_2oct(tag,f);
	ecrire_2oct(type,f);
	ecrire_4oct( nb_val,f);
	ecrire_4oct(val,f);
}

int init_tiff_file(FILE *tiff_file, uint32_t width, uint32_t length,
		   uint32_t MCU_height)
{
	if (tiff_file == NULL){
		return -1;
	}
	/* Header */
	ecrire_2oct(0x4949,tiff_file); //Little Endian
	ecrire_2oct(0x002a,tiff_file);
	ecrire_4oct(0x00000008,tiff_file);

        /* Nombre d'entrées (12) */
	ecrire_2oct(0x000c,tiff_file);

	/* Les entrées */
        ecrire_entree(0x0100,0x0003,0x00000001,width,tiff_file);
        ecrire_entree(0x0101,0x0003,0x00000001,length,tiff_file);
        ecrire_entree(0x0102,0x0003,0x00000003,0x0000009e,tiff_file);
        ecrire_entree(0x0103,0x0003,0x00000001,0x00000001,tiff_file);
        ecrire_entree(0x0106,0x0003,0x00000001,0x00000002,tiff_file);

	/* Offsets des différentes lignes de l'image */
	uint32_t nb_bande=ceil((float)length/(float)MCU_height);
	uint32_t taille_bande=3*MCU_height*width;

	if (nb_bande > 2.) {
		uint32_t adresse_offset = 0x000000b4 + 4*nb_bande;
        	ecrire_entree(0x0111,0x0004,nb_bande,adresse_offset,tiff_file);
	} else if (nb_bande == 2) {
       		ecrire_entree(0x0111,0x0003,0x00000002,0x00b40174,tiff_file);
	} else if (nb_bande == 1) {
		ecrire_entree(0x0111,0x0003,0x00000001,0x000000b4,tiff_file);
	}

        ecrire_entree(0x0115,0x0003,0x00000001,0x00000003,tiff_file);
        ecrire_entree(0x0116,0x0003,0x00000001,MCU_height,tiff_file);

	if (nb_bande > 2) {
        	ecrire_entree(0x0117,0x0004,nb_bande,0x000000b4,tiff_file);
	} else if (nb_bande == 2) {
        	ecrire_entree(0x0117,0x0004,0x00000002,
			      (taille_bande<<16)+taille_bande,tiff_file);
	} else if (nb_bande == 1) {
		ecrire_entree(0x0117,0x0004,0x00000001,taille_bande,tiff_file);
	}

        ecrire_entree(0x011a,0x0005,0x00000001,0x00a4,tiff_file);
        ecrire_entree(0x011b,0x0005,0x00000001,0x000000ac,tiff_file);
        ecrire_entree(0x0128,0x0003,0x00000001,0x00000002,tiff_file);

	/* Offset suivant */
	ecrire_4oct(0x00000000,tiff_file);

	/* BitsPerSamples */
	ecrire_2oct(0x0008,tiff_file);
	ecrire_2oct(0x0008,tiff_file);
	ecrire_2oct(0x0008,tiff_file);

	/* XResolution */
	ecrire_4oct(0x00000064,tiff_file);
	ecrire_4oct(0x00000001,tiff_file);
	//YResolution
	ecrire_4oct(0x00000064,tiff_file);
	ecrire_4oct(0x00000001,tiff_file);

	if (nb_bande > 2.) {
		uint32_t i=0;

		/* Taille en octets des différentes lignes */
		for (i=0;i<nb_bande;i++) {
			ecrire_4oct(taille_bande,tiff_file);
		}

		/* Offsets des différentes bandes */
		for (i=0;i<nb_bande;i++) {
			ecrire_4oct(0x00b4+8*nb_bande+i*taille_bande,tiff_file);
		}

	}

	/* En fait, il ne faut pas oublier le cas ou les dimensions en pixels de 
	 * l'image ne sont pas des multiples des dimensions de la MCU, dans ce cas
	 * il faut "caster" les type length, width et MCU_height afin de rajouter 
	 * une bande supplémentaire en bas 
	 */

        /* Initialisation des variables globales */
	width_image = width;
	MCU_h = MCU_height;
	cache = calloc(width_image * MCU_height * taille_pix, 1);
	cache_cour = cache;
	/* Pointeur sur la dernière composante du dernier pixel du cache */
	fin_cache = cache + width_image * MCU_height * taille_pix - 1;



	return 0;
}

/* Fonctions renvoyant les composantes RGB d'un pixel */

uint8_t getR(uint32_t pixel)
{
	return (pixel >> 16) & 0x000000FF;
}

uint8_t getG(uint32_t pixel)
{
	return (pixel >> 8) & 0x000000FF;
}

uint8_t getB(uint32_t pixel)
{
	return (pixel & 0x000000FF);
}

/* Copie la MCU passée en paramètre à write_tiff_file
 * dans le cache en les parcourant simultanément */
void copie_MCU(uint8_t largeur_a_ecrire, uint8_t largeur, uint8_t hauteur, uint32_t *ptr_MCU)
{
	for (uint32_t i = 0; i < hauteur; i++){
		for (uint8_t j = 0; j < largeur_a_ecrire; j++){
			cache_cour[3*j] = getR(ptr_MCU[i*largeur+j]);
			cache_cour[3*j+1] = getG(ptr_MCU[i*largeur+j]);
			cache_cour[3*j+2] = getB(ptr_MCU[i*largeur+j]);
		}
		cache_cour += (width_image) * taille_pix;
	}
}

/* Une fois le cache complètement rempli, celui-ci
 * est recopié dans le fichier tifffile */
void recopie_cache(uint8_t *cache, FILE *f)
{
	for (uint32_t i = 0; i < taille_pix * width_image * MCU_h; i++){
		ecrire_octet(cache[i],f);
	}
}

void write_tiff_file(FILE * tifffile , uint32_t w, uint32_t h, uint32_t * image)
{

	uint32_t debordement = width_image % w;
	uint32_t *ptr_MCU = image;

	/* 'reste' est la place (en largeur) restant dans le cache */
	uint32_t reste = cache + taille_pix * width_image - cache_cour;
	if (reste > taille_pix * w){
		copie_MCU(w,w,h,ptr_MCU);
		cache_cour = cache_cour - (MCU_h * width_image - w) * taille_pix;
	} else if (reste == taille_pix * w) {
		copie_MCU(w,w,h,ptr_MCU);
		recopie_cache(cache,tifffile);
		cache_cour = cache;
	} else {
		copie_MCU(debordement,w,h,ptr_MCU);
		recopie_cache(cache,tifffile);
		cache_cour = cache;
	}
}

void close_tiff_file(FILE * tifffile)
{
	free(cache);
	fclose(tifffile);
} 

void ecrire_MCU(FILE *tifffile, uint32_t w, uint32_t MCU_h, uint32_t color)
{
	uint32_t *MCU = calloc(w * MCU_h, sizeof(uint32_t));
	for (uint32_t i = 0; i < w * MCU_h; i++){
		MCU[i] = color;
	}
	write_tiff_file(tifffile,w,MCU_h,MCU);
}


#if 0
int main(void)
{
	printf("Test de init_tiff \n");
	int res;
	FILE *image = fopen("image_test.tiff","w");
	uint32_t width = 25;
	uint32_t length = 25;
	uint32_t MCU_height = 8;
	uint32_t MCU_width = 8;	
	uint32_t nb_MCU_bande=ceil((float)width/(float)MCU_height);
	res = init_tiff_file(image,width,length,MCU_height);


	if (res == 0 ) {
/*
		for (uint32_t i = 0; i<MCU_height*width*nb_bande; i++){
			ecrire_octet(0xff,image);
			ecrire_octet(0xc6,image);
			ecrire_octet(0x5f,image);
		}
*/
		ecrire_MCU(image,8,8,0x00ff0000); /* red */
		ecrire_MCU(image,8,8,0x0000ff00); /* green */
		ecrire_MCU(image,8,8,0x000000ff); /* blue */

		uint32_t *MCU = calloc(64, sizeof(uint32_t));
		for (uint32_t i = 0; i < 64; i++){
			MCU[i] = 0x00ffaacc;
		}
		copie_MCU(1,8,8,MCU); /* noir */
		recopie_cache(cache,image);
		cache_cour = cache;

		ecrire_MCU(image,8,8,0x00ff0000); /* red */ 
		ecrire_MCU(image,8,8,0x0000ff00); /* green */
		ecrire_MCU(image,8,8,0x000000ff); /* blue */

       
		copie_MCU(1,8,8,MCU); /* noir */
		recopie_cache(cache,image);
		cache_cour = cache;

		ecrire_MCU(image,8,8,0x00ff0000); /* red */
		ecrire_MCU(image,8,8,0x0000ff00); /* green */
		ecrire_MCU(image,8,8,0x000000ff); /* blue */

		copie_MCU(1,8,8,MCU); // noir
		recopie_cache(cache,image);
		cache_cour = cache;

		for (uint32_t i = 0; i<	MCU_height*nb_MCU_bande*MCU_width; i++){
			ecrire_octet(0xff,image);
			ecrire_octet(0xaa,image);
			ecrire_octet(0xcc,image);
		}


		printf("Création du fichier Tiff avec succès  \n");
	} else {
		printf("Erreur dans l'initialisation du fichier TIFF  \n");
	}

}
#endif
