#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>

#include "../include/jpeg.h"
#include "../include/inout.h"
#include "../include/analyse_header.h"
#include "../include/huffman.h"
#include "../include/unpack_block.h"
#include "../include/skip_segment.h"
#include "../include/iqzz.h"
#include "../include/idct.h"
#include "../include/upsampler.h"
#include "../include/conv.h"
#include "../include/tiff.h"


/* Decompresse un bloc en tenant compte de la resynchronisation */
void unpack_block_sync(FILE *f, scan_desc_t *scan, uint16_t RI, uint32_t i,
		      int32_t *bck_freq_qzz)
{
	static uint32_t nb_calls = 0;
	nb_calls++;
	unpack_block(f,scan,i,bck_freq_qzz);
	if (RI != 0 && nb_calls % RI == 0) {
		uint32_t cpt = 0;
		uint8_t oct = read_byte(&cpt,f);
		if (scan->window == 0xff) {
			assert(oct == 0x00);
			oct = read_byte(&cpt,f);
			scan->window = 0;
		}
		if (oct != 0xff) {
			printf("\n*** ERREUR: marqueur attendu (RST) ***\n");
			return;
		}
		oct = read_byte(&cpt,f);
		if (oct == 0xd9) {
			printbody("\nEnd of Image (EOI): fin du décodage\n");
			return;
		} else {
			uint8_t num = oct % 16;
			if (num == 0) {
				printbody("   Marqueur RST%u",num);
			} else if (num == 7) {
				printbody(" RST%u\n",num);
			} else {
				printbody(" RST%u",num);
			}
		}
		assert((oct % 16) == ((nb_calls / RI) - 1) % 8);
		scan->pred[0] = 0;
		scan->pred[1] = 0;
		scan->pred[2] = 0;
		scan->bit_count = 0;
	}
}		


/* Renvoie le nom du fichier TIFF a partir du nom du fichier JPEG */
char *tiff_name(char *path_to_jpeg_name)
{
	uint32_t last_dot = 0;
	uint32_t last_slash = 0;
	for (uint32_t i = 0; i < strlen(path_to_jpeg_name); i++) {
		if (path_to_jpeg_name[i] == '.') {
			last_dot = i;
		}
		if (path_to_jpeg_name[i] == '/') {
			last_slash = i+1;
		}
	}
	char *out = calloc(last_dot - last_slash + 6, sizeof(char));
	/* on recopie le nom du fichier sans l'extension ni le path */
	for (uint32_t i = last_slash; i < last_dot; i++) {
		out[i-last_slash] = path_to_jpeg_name[i];
	}
	strcat(out,".tiff");
	return out;
}
	

/* Calcule la taille des MCU d'une image im */
void compute_MCU_size(image_t *im, uint8_t MCU_size[2])
{
        printbody("Calcul de la taille d'une MCU...\n");
	MCU_size[0] = 0;
	MCU_size[1] = 0;
	for (uint8_t i = 0; i < im->nb_comp; i++) {
		if (MCU_size[0] < im->comp[i].echH) {
			MCU_size[0] = im->comp[i].echH;
		}
		if (MCU_size[1] < im->comp[i].echV) {
			MCU_size[1] = im->comp[i].echV;
		}
	}
	MCU_size[1] *= _BHEIGHT;
	MCU_size[0] *= _BWIDTH;
	printbody("    => MCU %ux%u\n", MCU_size[1], MCU_size[0]);
}


/* Decode un bloc frequentiel dans le fichier et renvoie un bloc pixmap */
void get_pixmap_block(uint8_t *pixmap_block, FILE *f, scan_desc_t *scan, 
		      uint8_t index, image_t *im, uint16_t RI, 
		      uint8_t QT[2][_BSIZE]) 
{
        /* decompression d'une MCU frequentielle zigzag quant. */
	printbody2("   │   ├─  Unpack block...\n");
	int32_t bck_freq_qzz[_BSIZE];
	unpack_block_sync(f,scan,RI,index,bck_freq_qzz);
	
        /* dequantification et dezigzag */
	printbody2("   │   ├─  IQZZ block...\n");
	int32_t bck_freq[_BSIZE];
	iqzz_block(bck_freq_qzz, bck_freq, QT[im->comp[index].iq]);
	
        /* conversion du domaine frequentiel vers bitmap */
	printbody2("   │   └─  IDCT block...\n");
	IDCT(bck_freq,pixmap_block);
}


/* 
 * decode une MCU pixmap RGB en lisant les MCU frequentielles correspondantes
 * (de chaque composante) dans le fichier JPEG
 */
void get_RGB_MCU(uint32_t *RGB_MCU, FILE *f, scan_desc_t *scan, image_t *im, 
		 uint16_t RI, uint8_t MCU_size[2], uint8_t QT[2][_BSIZE]) 
{
	uint8_t *YCbCr_MCU[3] = {NULL,NULL,NULL};
	for (uint32_t i = 0; i < im->nb_comp;i++) {
		uint32_t nb_block = im->comp[i].echV * im->comp[i].echH;
		printbody2(" Composante %u : %u bloc(s) a lire\n",
			   im->comp[i].ic,nb_block);
		uint8_t tab_bck[_BSIZE * nb_block];
		for (uint32_t j = 0; j < nb_block; j++) {
			printbody2("   ├── Bloc %u :\n",j+1);
			uint8_t bck[_BSIZE];
			get_pixmap_block(bck,f,scan,i,im,RI,QT);
			/* ajout du block dans le tableau pour sur-echant. */
			for (uint32_t k = 0; k < _BSIZE; k++) {
				tab_bck[_BSIZE * j + k] = bck[k];
			}
		}
		/* sur-echantillonnage */
		printbody2("   └── Upsampler...\n");
    		uint8_t h_factor = (MCU_size[0] / 8) / im->comp[i].echH;
		uint8_t v_factor = (MCU_size[1] / 8) / im->comp[i].echV;
		YCbCr_MCU[i] = calloc(MCU_size[0] * MCU_size[1],
				      sizeof(uint8_t *));
		upsampler(tab_bck,YCbCr_MCU[i],h_factor,v_factor,MCU_size[0]/8,
			  MCU_size[1]/8);
	}
	/* conversion YCbCr vers ARGB */
	printbody2(" => Conversion vers RGB...\n");
	if (im->nb_comp == 3) {
		YCbCr_to_ARGB(YCbCr_MCU,RGB_MCU,MCU_size[0]/8,MCU_size[1]/8);
	} else if (im->nb_comp == 1) {
		for (uint32_t k = 0; k < MCU_size[0]*MCU_size[1]; k++) {
			RGB_MCU[k] = YCbCr_MCU[0][k]*(1+256+65536);
		}
	}
	for (uint32_t i = 0; i < im->nb_comp;i++) {
		free(YCbCr_MCU[i]);
	}
}



/* PROCEDURE PRINCIPALE */

int32_t main(int32_t argc, char *argv[])
{
	if (argc < 2) {
		printf("Usage: %s <nom_de_fichier_jpeg>\n",argv[0]);
		return 0;
	}
	
	FILE * f;
	f = fopen(argv[1],"r");
	if (f == 0) {
		printf("Fichier inexistant ou nom incorrect.\n");
		printf("Conversion du fichier '%s' échouée.\n",argv[1]);
		return -1;
	}

	if (_DISPLAY_HEAD >= 1 || _DISPLAY_BODY >= 1 || _DRY_RUN) {
		printf("\n*********** DECODAGE DE '%s' ***********\n",argv[1]);
	}
			
    /* I. Decodage de l'en-tete*/

	/* Initialisation des variables */
	image_t *im = malloc(sizeof(image_t)); /* Proprietes de l'image */
	uint16_t RI = 0;                       /* Restart Interval */
	uint8_t QT[2][_BSIZE];                 /* Tables de quantification */
	huff_table_t *HT[2][2] = {{NULL,NULL},{NULL,NULL}}; /* T. de Huffman */
	
	printmark("\n+========== HEADER ===============================+\n");
	printmark("octet :  description\n\n");
	
	/* Decodage */
	uint32_t cpt = 1;
	read_byte(&cpt,f);
	int8_t check = analyse_header(im,&RI,QT,HT,&cpt,f);
	if (check == -1) {
		printf("Conversion du fichier '%s' échouée.\n",argv[1]);
		fclose(f);
		printbody("Fichier JPEG fermé\n");
		return -1;
	}

	printmark("+========== END OF HEADER ========================+\n\n");

	if (_DISPLAY_HEAD == 0) {
		printbody("Analyse de l'en-tête\n");
	}

    /* II. Calculs preliminaires */
	
	/* Calcul de la taille d'une MCU */
	uint8_t MCU_size[2];
	compute_MCU_size(im, MCU_size);
	const uint32_t nb_pxl_in_MCU = MCU_size[0] * MCU_size[1]; 
	printbody2("    => Nb px par MCU : %u\n",nb_pxl_in_MCU);

	/* Calcul du nombre de MCU dans l'image */
	uint32_t nb_MCU_width = im->width / MCU_size[0] 
		+ ((im->width % MCU_size[0]) != 0);
	uint32_t nb_MCU_height = im->height / MCU_size[1] 
		+ ((im->height % MCU_size[1]) != 0);
	printbody("Taille de l'image en MCU : %ux%u\n",nb_MCU_height,
		  nb_MCU_width);
	uint32_t nb_RGB_MCU = nb_MCU_width * nb_MCU_height;
	printbody("Nb de MCU RGB : %u\n",nb_RGB_MCU);
	
	/* Calcul de l'intervalle de resync en nb de blocs */
	uint32_t blocks_per_MCU = 0;
	for (uint32_t i = 0; i < im->nb_comp; i++) {
		blocks_per_MCU += im->comp[i].echH * im->comp[i].echV;
	}
	uint32_t RI_block = blocks_per_MCU * RI;
	if (RI != 0) {
		printbody("Nombre de blocs par MCU : %u\n",blocks_per_MCU);
		printbody("Intervalle de resync en blocs : %u blocs\n",RI_block);
	}

	if (_DRY_RUN) {
		display_data_and_EOI(&cpt,f);
		goto end;
	}

	printbody("\n+========== BODY =================================+\n");
	
	/* Initialisation du scan */
	printbody("Initialisation du scan\n");
	scan_desc_t scan;
	scan.bit_count = 0;
	scan.window = 0;
	scan.pred[0] = 0;
	scan.pred[1] = 0;
	scan.pred[2] = 0;
	for (uint32_t i = 0; i < im->nb_comp; i++) {
		scan.table[0][i] = HT[0][im->comp[i].ih_DC];
		scan.table[1][i] = HT[1][im->comp[i].ih_AC];
	}
	
	/* Initialisation du fichier TIFF */
	char *tiff_file_name = tiff_name(argv[1]);
	printbody("Création du fichier '%s'\n",tiff_file_name);
	FILE *tiff_f = fopen(tiff_file_name,"w");
	init_tiff_file(tiff_f,im->width,im->height,MCU_size[1]);


    /* III. Decodage des donnees brutes */

	/* reconstruction des MCU*/
	printbody("Reconstitutions des MCU (decodage des donnees brutes)\n");
	uint32_t *RGB_MCU = calloc(nb_pxl_in_MCU,sizeof(uint32_t));
	for (uint32_t i = 0; i < nb_RGB_MCU; i++) {
		if (RI == 0 && nb_RGB_MCU > 10 && i % (nb_RGB_MCU/10) == 0) {
			printbody1("    %3u%%\n",i*100/nb_RGB_MCU);
		}
		get_RGB_MCU(RGB_MCU,f,&scan,im,RI_block,MCU_size,QT);
		printRGBMCU(RGB_MCU,MCU_size);
		printbody2("\n");
		write_tiff_file(tiff_f,MCU_size[0],MCU_size[1],RGB_MCU);
	}
	free(RGB_MCU);

	/* lecture du marqueur de fin d'image EOI */
	if (RI == 0) {
		uint16_t mark = read_2bytes(&cpt,f);
		assert(mark == 0xffd9);
		printbody("Marqueur de fin d'image EOI : Fin du décodage\n");
	}
	printbody("+========== END OF BODY ==========================+\n\n");

    /* fin de procedure */
	printf("Conversion du fichier '%s'\nen '%s' réussie.\n",argv[1],
	       tiff_file_name);
	free(tiff_file_name);
	close_tiff_file(tiff_f);
	printbody("Fichier TIFF fermé\n");
end:
	if (_DRY_RUN) {
		printf("+========== END OF BODY ==========================+\n");
		printf("** Attention : mode Dry-Run actif **\n");
		printf("Lecture du fichier terminée avec succès\n");
	}
	fclose(f);
	printbody("Fichier JPEG fermé\n");
	free(im->comp);
	free(im);
	free_huffman_tables(HT[0][0]);
	free_huffman_tables(HT[1][0]);
	free_huffman_tables(HT[0][1]);
	free_huffman_tables(HT[1][1]);
	printbody("Mémoire libérée\n");
	return 0;
}
