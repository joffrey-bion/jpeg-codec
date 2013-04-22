#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>

#include "../include/jpeg.h"
#include "../include/inout.h"
#include "../include/analyse_header.h"

//#include "../include/iqzz.h"
#include "../include/idct.h"
#include "../include/upsampler.h"
#include "../include/conv.h"
#include "../include/tiff.h"

//#include "../include/enc-qzz.h"
#include "../include/enc-dct.h"
#include "../include/enc-downsampler.h"
#include "../include/enc-conv.h"
#include "../include/enc-tiff.h"


/* Renvoie le nom du fichier TIFF de sortie a partir du nom d'entree */
char *out_name(char *path_to_tiff_name)
{
	uint32_t last_dot = 0;
	uint32_t last_slash = 0;
	for (uint32_t i = 0; i < strlen(path_to_tiff_name); i++) {
		if (path_to_tiff_name[i] == '.') {
			last_dot = i;
		}
		if (path_to_tiff_name[i] == '/') {
			last_slash = i+1;
		}
	}
	char *out = calloc(last_dot - last_slash + 10, sizeof(char));
	/* on recopie le nom du fichier sans l'extension ni le path */
	for (uint32_t i = last_slash; i < last_dot; i++) {
		out[i-last_slash] = path_to_tiff_name[i];
	}
	strcat(out,"-out.tiff");
	return out;
}

void print_block(uint8_t *block)
{
	for (uint32_t i = 0; i<8; i++) {
		for (uint32_t j = 0; j<8; j++) {
			printf("%2u ",block[8*i+j]);
		}
		printf("\n");
	}
}

/* encode puis redecode un bloc pixmap */
void change_pixmap_block(uint8_t *in_block, uint8_t *out_block) 
{
	for (uint32_t k = 0; k < _BSIZE; k++) {
		out_block[k] = in_block[k];
	}
}


/* 
 * decode une MCU pixmap RGB en lisant les MCU frequentielles correspondantes
 * (de chaque composante) dans le fichier JPEG
 */
void change_RGB_MCU(uint32_t *in_RGB_MCU, uint32_t *out_RGB_MCU, image_t *im, 
		    uint8_t MCU_size[2], uint8_t quality) 
{
	/* conversion ARGB vers YCbCr */
	printbody2(" => Conversion vers YCbCr...\n");
	uint8_t *YCbCr_MCU[3] = {NULL,NULL,NULL};
	for (uint32_t i = 0; i < im->nb_comp; i++) {
		YCbCr_MCU[i] = calloc(MCU_size[0] * MCU_size[1],
				      sizeof(uint8_t *));
	}
	ARGB_to_YCbCr(in_RGB_MCU,YCbCr_MCU,MCU_size[0]/8,MCU_size[1]/8);


	for (uint32_t i = 0; i < im->nb_comp; i++) {
		uint32_t nb_block = im->comp[i].echV * im->comp[i].echH;
		printbody2(" Composante %u : %u bloc(s) a creer\n",
			   im->comp[i].ic,nb_block);
		
		/* sous-echantillonnage */
		printbody2("   ├── downsampler...\n");
    		uint8_t tab_bck[_BSIZE * nb_block];
		uint8_t tab_bck2[_BSIZE * nb_block];
		uint8_t h_factor = (MCU_size[0] / 8) / im->comp[i].echH;
		uint8_t v_factor = (MCU_size[1] / 8) / im->comp[i].echV;
		downsampler(YCbCr_MCU[i],tab_bck,h_factor,v_factor,MCU_size[0]/8,
			  MCU_size[1]/8);

		for (uint32_t j = 0; j < nb_block; j++) {
			printbody2("   ├── Bloc %u :\n",j+1);
			
			/* extraction du bloc */
			uint8_t bck[_BSIZE];
			for (uint32_t k = 0; k < _BSIZE; k++) {
				bck[k] = tab_bck[_BSIZE * j + k];
			}

			/* ENCODAGE / DECODAGE du bloc */
			uint8_t bck2[_BSIZE];
			change_pixmap_block(bck,bck2);
		
			/* remplacement du bloc */
			for (uint32_t k = 0; k < _BSIZE; k++) {
				tab_bck2[_BSIZE * j + k] = bck2[k];
			}
		}

		/* sur-echantillonnage */
		printbody2("   └── Upsampler...\n");
    		YCbCr_MCU[i] = calloc(MCU_size[0] * MCU_size[1],
				      sizeof(uint8_t *));
		upsampler(tab_bck2,YCbCr_MCU[i],h_factor,v_factor,MCU_size[0]/8,
			  MCU_size[1]/8);
	}

	/* conversion YCbCr vers ARGB */
	printbody2(" => Conversion vers RGB...\n");
	YCbCr_to_ARGB(YCbCr_MCU,out_RGB_MCU,MCU_size[0]/8,MCU_size[1]/8);
	for (uint32_t i = 0; i < im->nb_comp;i++) {
		free(YCbCr_MCU[i]);
	}
}



/* PROCEDURE PRINCIPALE */

int32_t main(int32_t argc, char *argv[])
{
	if (argc < 2) {
		printf("Usage: %s <nom_de_fichier_tiff>\n",argv[0]);
		return 0;
	}
	
	FILE * f;
	f = fopen(argv[1],"r");
	if (f == 0) {
		printf("Fichier inexistant ou nom incorrect.\n");
		printf("Conversion du fichier '%s' échouée.\n",argv[1]);
		return -1;
	}

	if (_DISPLAY_HEAD >= 1 || _DISPLAY_BODY >= 1) {
		printf("\n*********** DECODAGE DE '%s' ***********\n",argv[1]);
	}

			
    /* I. Decodage de l'en-tete TIFF */

	/* Initialisation des variables */
	uint8_t quality = 3;
	uint8_t QT[2][_BSIZE];                 /* Tables de quantification */
	uint8_t MCU_size[2];
	image_t *im = malloc(sizeof(image_t)); /* Proprietes de l'image */
	im->nb_comp = 3;
	im->comp = calloc(3,sizeof(comp_t));
	im->comp[0].ic = 1;
	im->comp[1].ic = 2;
	im->comp[2].ic = 3;
	im->comp[0].iq = 0;
	im->comp[1].iq = 1;
	im->comp[2].iq = 1;
	

	printmark("\n+========== HEADER ===============================+\n");
	
	/* Decodage */
	read_tiff_file_header(f,&(im->height),&(im->width),&(MCU_size[1]));
	MCU_size[0] = 16;

	im->comp[0].echH = MCU_size[0]/8;
	im->comp[0].echV = MCU_size[1]/8;
	im->comp[1].echH = 1;
	im->comp[1].echV = 1;
	im->comp[2].echH = 1;
	im->comp[2].echV = 1;

	printmark("+========== END OF HEADER ========================+\n\n");

	if (_DISPLAY_HEAD == 0) {
		printbody("Analyse de l'en-tête\n");
	}


    /* II. Calculs preliminaires */
	
	/* Calcul du nombre de MCU dans l'image */
	uint32_t nb_MCU_width = im->width / MCU_size[0] 
		+ ((im->width % MCU_size[0]) != 0);
	uint32_t nb_MCU_height = im->height / MCU_size[1] 
		+ ((im->height % MCU_size[1]) != 0);
	printbody("Taille de l'image en MCU : %ux%u\n",nb_MCU_height,
		  nb_MCU_width);
	uint32_t nb_RGB_MCU = nb_MCU_width * nb_MCU_height;
	printbody("Nb de MCU RGB : %u\n",nb_RGB_MCU);
	
	
	printbody("\n+========== BODY =================================+\n");
	
	/* Initialisation du fichier TIFF */
	char *out_file = out_name(argv[1]);
	printbody("Création du fichier '%s'\n",out_file);
	FILE *out_f = fopen(out_file,"w");
	init_tiff_file(out_f,im->width,im->height,MCU_size[1]);



    /* III. Decodage des donnees brutes */

	/* reconstruction des MCU*/
	printbody("Reconstitutions des MCU (decodage des donnees brutes)\n");
	uint32_t nb_pxl_in_MCU = MCU_size[0] * MCU_size[1];
	uint32_t *out_RGB_MCU = calloc(nb_pxl_in_MCU,sizeof(uint32_t));
	uint32_t *in_RGB_MCU = calloc(nb_pxl_in_MCU,sizeof(uint32_t));
	for (uint32_t i = 0; i < nb_RGB_MCU; i++) {
		if (nb_RGB_MCU > 10 && i % (nb_RGB_MCU/10) == 0) {
			printbody1("    %3u%%\n",i*100/nb_RGB_MCU);
		}
		get_tiff_MCU(f,MCU_size[0],in_RGB_MCU);
		change_RGB_MCU(in_RGB_MCU,out_RGB_MCU,im,MCU_size,quality);
		printbody2("\n");
		write_tiff_file(out_f,MCU_size[0],MCU_size[1],out_RGB_MCU);
	}
	free(in_RGB_MCU);
	free(out_RGB_MCU);
	
	printbody("+========== END OF BODY ==========================+\n\n");


    /* fin de procedure */
	printf("Conversion du fichier '%s'\nen '%s' réussie.\n",argv[1],
	       out_file);
	free(out_file);
	close_tiff_file(out_f);
	printbody("Fichier TIFF fermé\n");

	if (_DRY_RUN) {
		printf("+========== END OF BODY ==========================+\n");
		printf("** Attention : mode Dry-Run actif **\n");
		printf("Lecture du fichier terminée avec succès\n");
	}
	fclose(f);
	printbody("Fichier JPEG fermé\n");
	free(im->comp);
	free(im);
	printbody("Mémoire libérée\n");
	return 0;
}




#if 0	
	/* conversion du domaine frequentiel vers bitmap */
	printbody2("   │   ├─  DCT block...\n");
	int32_t bck_freq1[_BSIZE];
	DCT(in_block,bck_freq1);

       /* quantification et zigzag */
	printbody2("   │   ├─  IQZZ block...\n");
	uint8_t QT[_BSIZE];
	int32_t bck_freq_qzz[_BSIZE];
	qzz_block(bck_freq, bck_freq_qzz, QT, quality);

/* CHANGEMENT DE SENS => DECODAGE */

	/* dequantification et dezigzag */
	printbody2("   │   ├─  IQZZ block...\n");
	int32_t bck_freq2[_BSIZE];
	iqzz_block(bck_freq_qzz, bck_freq2, QT);

        /* conversion du domaine frequentiel vers bitmap */
	printbody2("   │   └─  IDCT block...\n");
	IDCT(bck_freq2,out_block);

#endif	
