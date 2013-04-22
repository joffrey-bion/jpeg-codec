#include "../include/enc-tiff.h"

/* Variables globales */ 

uint32_t length,width,nb_bande; /* dimensions et nombre de bandes de l'image */
uint32_t MCU_height;
uint32_t offset_un,offset1,offset2,offset,offset_bande;
/* Précision des différents offsets (adresses des bandes) 
 * Nb_bande = 1 -- offset_un
 * Nb_bande = 2 -- offset1 et offset2
 * Nb_bande > 2 -- offset
 * offset_bande -- offset de la bande traitée
 */
uint32_t taille_bande1,taille_bande2; /* taille des bandes quand Nb_bande = 2 */
uint32_t taille_adresse; /* Précise sur combien d'octets sont les adresses */
uint8_t Endian; /* Précise la manière de lire : little/big endian */
uint8_t position_curseur = 0; /* Précise la position de la MCU % à la bande */

#if 0
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
#endif

uint16_t read_2bytes_endian(uint32_t *cpt, FILE *f, uint8_t Endian)

{
	if ( Endian == 0 ) { // BigEndian
		uint16_t oct1 = 0;
		uint16_t oct2 = 0;
		oct1 = read_byte(cpt,f);
		oct2 = read_byte(cpt,f);
		return (oct1 << 8) + oct2;
	} else if ( Endian == 1 ) { // LittleEndian
		uint16_t oct1 = 0;
		uint16_t oct2 = 0;
		oct1 = read_byte(cpt,f);
		oct2 = read_byte(cpt,f);
		return (oct2 << 8) + oct1;
	}
	return 0;

}

uint32_t read_4bytes_endian(uint32_t *cpt, FILE *f,uint8_t Endian)
{
	if ( Endian == 0 ) { // BigEndian
		uint32_t oct1 = 0;
		uint32_t oct2 = 0;
		oct1 = read_2bytes_endian(cpt,f,Endian);
		oct2 = read_2bytes_endian(cpt,f,Endian);
		return (oct1 << 16) + oct2;
	} else if ( Endian == 1 ) { // LittleEndian
		uint8_t oct1,oct2,oct3,oct4;
		oct1 =read_byte(cpt,f);
		oct2 =read_byte(cpt,f);
		oct3 =read_byte(cpt,f);
		oct4 =read_byte(cpt,f);
		return (oct4 << 24) + (oct3 << 16) + (oct2 << 8) + oct1;
	}
	return 0;

}


/* Fonction qui renvoie les composantes RGB sur 32 bits */
uint32_t RGB(FILE *f)
{
	uint32_t cpt = 0;
	uint32_t R = 0;
	uint32_t G = 0;
	uint32_t B = 0;
	R = read_byte(&cpt,f);
	G = read_byte(&cpt,f);
	B = read_byte(&cpt,f);
	return (R << 16) + (G << 8) + B;
}

void lire_type(uint32_t *cpt, FILE *f)
{
	uint16_t cour1;
	cour1 = read_4bytes_endian(cpt,f,Endian);
	if (cour1 == 0x0001) {
		printf("Type: Byte ||");
	} else if (cour1 == 0x0002 ) {
		printf("Type: ASCII ||");
	} else if (cour1 == 0x0003 ) {
		printf("Type: SHORT ||");
	} else if (cour1 == 0x0004 ) {
		printf("Type: LONG ||");
	} else if (cour1 == 0x0005 ) {
		printf("Type: RATIONAL ||");
	}
}


int read_tiff_file_header(FILE *tiff_file, uint16_t *im_height, 
			  uint16_t *im_width, uint8_t *MCU_h)
{
	if (tiff_file == NULL){
		return -1;
	}

	uint16_t nb_entree;
	uint32_t cpt = 0;
	uint32_t cour1;
	uint32_t cour2,cour3,cour4;
	uint16_t cour = read_2bytes_endian(&cpt,tiff_file,Endian);

	printf("\n ***** TIFF HEADER *****\n \n");
	
	if (cour == 0x4D4D) {
		printf(" Endianess du fichier : BigEndian \n" );
		Endian = 0;
	} else if (cour == 0x4949 ){
		printf(" Endianess du fichier : LittleEndian \n" );
		Endian = 1;
	} else {
		printf(" Erreur du fichier tiff \n");
		return -1;
	}
	
	cour = read_2bytes_endian(&cpt,tiff_file,Endian);

	if (cour == 0x002a) {
		printf(" Identification du TIFF : 42  Ok \n" );
	} else {
		printf(" Erreur Identification du TIFF =/ 42 \n");
		return -1;
	}

	cour4 = read_4bytes_endian(&cpt,tiff_file,Endian);
	printf(" Pointeur sur le premier IFD : %u \n",cour4);
	
	/* IFD */

	cour = read_2bytes_endian(&cpt,tiff_file,Endian);
	nb_entree = cour;
	printf(" Nombres d'entrées : %u \n",nb_entree);

	for (int i=0 ; i < nb_entree+2 ; i++) {
		/* Tag d'identification */
		cour = read_2bytes_endian(&cpt, tiff_file,Endian);  
		/* Type */
		cour1 = read_2bytes_endian(&cpt, tiff_file,Endian); 
		/* Nombre de valeur associées à l'entrée */ 
		cour2 = read_4bytes_endian(&cpt, tiff_file,Endian); 
		
		if (cour == 0x100) {
			cour3 = read_4bytes_endian(&cpt, tiff_file,Endian); 
			printf(" Nb Colonnes Image : %u \n",cour3);
			width = cour3;
			*im_width = width;
		} else if (cour == 0x101 ) {
			cour3 = read_4bytes_endian(&cpt, tiff_file,Endian); 
			printf(" Nb Lignes Image : %u \n",cour3);
			length = cour3; // On recupère length
			*im_height = length;
		} else if (cour == 0x102 ) {
			cour3 = read_4bytes_endian(&cpt, tiff_file,Endian); 
			fseek(tiff_file,cour3,SEEK_SET);
			uint16_t coura,courb,courc;
			coura = read_2bytes_endian(&cpt, tiff_file,Endian);
			courb = read_2bytes_endian(&cpt, tiff_file,Endian);
			courc = read_2bytes_endian(&cpt, tiff_file,Endian);
			printf(" Taille d'une composante couleur : %u %u %u \n",
			       coura,courb,courc);
			fseek(tiff_file,cpt-2,SEEK_SET);
		} else if (cour == 0x103 ) {
			cour3=read_4bytes_endian(&cpt, tiff_file,Endian); 
			if (cour3 == 1 ){
				printf(" Compression : Pas de compression \n");
			} else if (cour3 == 2) {
				printf(" Compression : Huffman \n");
			} else if (cour3 == 32773) {
				printf(" Compression : PackBit \n");
			}
		} else if (cour == 0x106 ) {
			cour3=read_4bytes_endian(&cpt, tiff_file,Endian); 
			if (cour3 == 2 ){
				printf(" Image en RGB : %u \n",cour);
			}
		} else if (cour == 0x111 ) {
			if (cour1 == 0x0003) {
				taille_adresse=2;
			} else if (cour1 == 0x0004) {
				taille_adresse=4;
			}
			printf(" Taille en octets des adresses : %u \n",
			       taille_adresse);
			printf(" Nombre de bandes : %u \n",cour2);
			nb_bande=cour2;
			if (nb_bande == 1 ) {
				offset_un = read_4bytes_endian(&cpt, tiff_file,
							       Endian);	
				printf(" Offsets de l'unique bande : 0x%x \n",
				       offset_un);
			} else if ( nb_bande == 2 ) {
				offset1 = read_2bytes_endian(&cpt, tiff_file,
							   Endian);
				offset2 = read_2bytes_endian(&cpt, tiff_file,
							   Endian);
				printf(" Offsets de la première bande : 0x%x \n"                                       ,offset2);
				printf(" Offsets de la deuxième bande : 0x%x \n"                                       ,offset1);
			} else {
				cour3 = read_4bytes_endian(&cpt, tiff_file,
							   Endian);
				offset = cour3;	
				printf(" Offsets des bandes : 0x%x \n",offset);
			}
		} else if (cour == 0x115 ) {
			cour3=read_4bytes_endian(&cpt, tiff_file,Endian); 
			printf(" Nb de composantes par pixel : %u \n",cour3);

		} else if (cour == 0x116 ) {
			cour3=read_4bytes_endian(&cpt, tiff_file,Endian); 
			printf(" Hauteur en pixels des lignes TIFF - MCU_height : %u \n",cour3);
			MCU_height=cour3;
			*MCU_h = MCU_height;
		} else if (cour == 0x117 ) {
			printf(" Nombre de bandes : %u \n",cour2);
			if (nb_bande == 1 ) {
				cour3=read_4bytes_endian(&cpt,tiff_file,Endian);
				offset_bande = cour3;
				printf(" Taille de l'unique bande : %u \n",
				       offset_bande);
			} else if ( nb_bande == 2 ) {
				cour1=read_2bytes_endian(&cpt,tiff_file,Endian);
				taille_bande1 = cour1; 
				printf(" Taille de la première bande : %u \n",
				       taille_bande1);
				cour1=read_2bytes_endian(&cpt,tiff_file,Endian);
				taille_bande2 = cour1;
				printf(" Taille de la deuxième bande : %u \n"
				       ,taille_bande2);
			} else {
				cour3=read_4bytes_endian(&cpt,tiff_file,Endian);
				offset_bande = cour3;	
				printf(" Taille des bandes à l'adresse : 0x%x \n",offset_bande);
			}
		} else if (cour == 0x11a ) {
			cour3 = read_4bytes_endian(&cpt, tiff_file,Endian); 
			printf(" Nombres de pixels par unités de mesure (horizontal) : %u \n",cour3);
		} else if (cour == 0x11b ) {
			cour3 = read_4bytes_endian(&cpt, tiff_file,Endian); 
			printf(" Nombres de pixels par unités de mesure (vertical) : %u \n",cour3);
		} else if (cour == 0x128 ) {
			cour3 = read_4bytes_endian(&cpt, tiff_file,Endian); 
			if (cour3 == 1 ){
				printf(" Unité de résolution : Pas d'unité \n");
			} else if (cour3 == 2 ) {
				printf(" Unité de résolution : Centimètres \n");
			} else if (cour3 == 3 ) {
				printf(" Unité de résolution : Pouces \n");
			}
		} else if (cour == 0x131 ) {
			cour3 = read_4bytes_endian(&cpt, tiff_file,Endian); 
			printf(" Application");
		}
	}
	cour3 = read_4bytes_endian(&cpt, tiff_file,Endian); /* Offset suivant */
	if (cour3 == 0 ) {
		printf(" Pas d'IFD supplémentaire \n" );
	}
     
	return 0; 
}

/* Procédure permettant de récupérer la MCU désignée par
 * la variable "position_curseur" dans la bande d'adresse
 * "offset_debut_bande" */
void traiter_bande(uint32_t offset_debut_bande, FILE *tiff_file, uint32_t w,
		   uint32_t *MCU_us,uint32_t *nb_MCU_traitee,
		   uint32_t nb_MCU_bande )
{
        /* nombre de MCU déjà traités dans la bande */
	uint32_t nb_MCU_TB = (*nb_MCU_traitee % nb_MCU_bande);
        /* position du curseur où on pointe sur la MCU à renvoyer */
	uint32_t position_curseur = offset_debut_bande + nb_MCU_TB * 3 * w;
	uint32_t adresse,adresse_un; /* stocke l'adresse des lignes de la MCU */

	if ( fseek(tiff_file,offset_un,SEEK_SET) == 0 ) {
                /* Si cela concerne la dernière MCU "partielle" */ 
		if (((*nb_MCU_traitee % nb_MCU_bande) == nb_MCU_bande-1)
		    && (width % w != 0) ) {
			uint32_t k=0;
			for (uint32_t i = 0; i < MCU_height ; i++) {
				adresse = position_curseur + i * width * 3;
				fseek(tiff_file,adresse,SEEK_SET);
				for (uint32_t j = 0; j < (width % w) ; j++) {
					MCU_us[k] = RGB(tiff_file);
					k++;
				}
				uint32_t der_col = MCU_us[k-1];
                                /* on complète la MCU en répétant 
                                 * la dernière colonne */
				for (uint32_t j = (width % w); j < w ; j++) {
					MCU_us[k] = der_col;
					k++;
				}
			}	
		} else {
			uint32_t k = 0;
			for (uint32_t i = 0; i < MCU_height ; i++) 
			{       
				adresse_un = position_curseur + i * width * 3;
				fseek(tiff_file,adresse_un,SEEK_SET);
				for (uint32_t j = 0; j < w ; j++) {
					MCU_us[k] = RGB(tiff_file);		
					k++;
				}
			}
		}
	} else {
		printf("Erreur dans la lecture du fichier");
	}
	(*nb_MCU_traitee)++;

}

/* procédure qui renvoie à chaque appel la MCU suivante de l'image */
void get_tiff_MCU(FILE *tiff_file, uint32_t w, uint32_t *MCU_us )
{

uint32_t *cpt = malloc(sizeof(uint32_t));

        /* Nombre de MCU totale traitées */
	static uint32_t nb_MCU_traitees = 0;
        /* Nombre de MCU dans la bande */
	uint8_t nb_MCU_bande = ceil(((float)width)/((float)w));
        /* Adresses des bandes */
	uint16_t cour2 = 0;
	uint32_t cour4 = 0;

	if (nb_bande == 1) {
		traiter_bande(offset_un,tiff_file, w, MCU_us,
				&nb_MCU_traitees, nb_MCU_bande );
	} else if (nb_bande == 2) {
		if (nb_MCU_traitees < nb_MCU_bande ) {
			traiter_bande(offset1,tiff_file, w, MCU_us,
				&nb_MCU_traitees, nb_MCU_bande );
		} else { 
			traiter_bande(offset2,tiff_file, w, MCU_us,
				&nb_MCU_traitees, nb_MCU_bande );
		}
	} else if (nb_bande > 2) {
		uint32_t nb_bande_remplies = nb_MCU_traitees/nb_MCU_bande;
		fseek(tiff_file,offset,SEEK_SET);
		if ( taille_adresse == 2 ) {
			for (uint32_t i=0; i < nb_bande_remplies+1 ; i++) {
				cour2=read_2bytes_endian(cpt,tiff_file,Endian);
			}
			traiter_bande(cour2,tiff_file,w, MCU_us,
				      &nb_MCU_traitees,nb_MCU_bande );
		} else if ( taille_adresse == 4 ) {
			for (uint32_t i = 0; i < nb_bande_remplies+1 ; i++) {
				cour4=read_4bytes_endian(cpt,tiff_file,Endian);
			}
			traiter_bande(cour4,tiff_file, w, MCU_us,
				      &nb_MCU_traitees, nb_MCU_bande );
		}
		
	}

}


#if 0

void affiche_MCU(FILE *tiff_file, uint32_t *MCU, uint32_t w, uint32_t j)
{
		printf("\n * MCU n°%u * :\n\n",j);
		get_tiff_MCU(tiff_file,w,MCU);
		for (uint32_t i=0; i < w*MCU_height; i++) {
			printf("0x%06x ",MCU[i]);
			if ((i+1) % w == 0) {
			printf("\n");
			}
		}
}

/* procédure qui affiche toutes les MCU du fichier */
void test(FILE *tiff_file, uint32_t w){
	uint16_t im_h;
	uint16_t im_w;
	uint8_t MCU_h;
	read_tiff_file_header(tiff_file,&im_w,&im_h,&MCU_h);
	uint8_t nb_MCU_bande = ceil(((float)width)/((float)w));
	for (uint32_t i=0; i < nb_bande; i++) {
		printf("\n *** Bande numéro %i ***\n", i+1);
		for (int j=0; j < nb_MCU_bande; j++){
			uint32_t *MCU = calloc(w*MCU_height,sizeof(uint32_t));
			affiche_MCU(tiff_file,MCU,w,j+1);
			free(MCU);
		}
	}
}


int main(void) 
{
	
#if 0
	/* test pour 2 bandes */
	uint32_t w = 8;
	FILE *tiff_file=fopen("../fichiers_test/8x24.tiff","r");
	assert(tiff_file != NULL);
	test(tiff_file,w);
	fclose(tiff_file);
#endif

#if 0
        /* test pour + de 2 bandes avec débordement */
	uint32_t w2 = 8;
	FILE *tiff_file2 = fopen("../fichiers_test/image_test.tiff","r");
	assert(tiff_file2 != NULL);
        /* image_test.tiff est l'image représentant un drapeau
         * rouge-vert-bleu avec un débordement rose pour
         * tester les cas limites */
	test(tiff_file2,w2);
	fclose(tiff_file2);
#endif

#if 0
	/* test sur une image complète */
	uint32_t w3 = 16;
	FILE *tiff_file3 = fopen("../zorglub.tiff","r");
	assert(tiff_file3 != NULL);
        /* image_test.tiff est l'image représentant un drapeau
         * rouge-vert-bleu avec un débordement rose pour
         * tester les cas limites */
	uint16_t im_h;
	uint16_t im_w;
	uint8_t MCU_h;
	read_tiff_file_header(tiff_file3,&im_w,&im_h,&MCU_h);
        /* test(tiff_file3,w3); */
	fclose(tiff_file3);

#endif
	return 0;
}

#endif
