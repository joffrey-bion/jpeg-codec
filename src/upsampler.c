#include "../include/upsampler.h"

/* 
 * Remarque : On se représente *MCU_ds comme un tableau de taille
 * nb_block_V x nb_block_H contenant des MCU de taille 
 * _BWIDTH x _BHEIGHT alors qu'en réalité *MCU_ds est 
 * grand tableau a une dimension 
 */

/* 
 * Etire le pixel (l,c) du bloc (p,q) de h_factor fois vers la droite
 * et de v_factor fois vers le bas (en le dupliquant aux bons 
 * endroits dans le tableau de sortie *MCU_us à partir de l'indice k 
 */
void etire_pix(uint8_t *MCU_ds,uint8_t *MCU_us,uint8_t h_factor,
	       uint8_t v_factor,uint16_t nb_block_H,uint8_t p,
	       uint8_t l,uint8_t q,uint8_t c, uint8_t k) 
{
	uint8_t longueur_ligne = nb_block_H * _BWIDTH * h_factor;
        /* position du pixel en question dans le tableau *MCU_ds */
	uint32_t pos = c+q*_BSIZE+l*_BWIDTH+p*_BSIZE*nb_block_H; 
        /* étirement : */
	for (uint8_t h = 0; h < h_factor; h++){
		for (uint8_t v = 0; v < v_factor; v++){	
			MCU_us[k+h+v*longueur_ligne]=MCU_ds[pos];
		}
	}
}

/* Procède à l'étirement sur toute une ligne (l fixé) */
void etire_ligne(uint8_t *MCU_ds,uint8_t *MCU_us,uint8_t h_factor,
		 uint8_t v_factor,uint16_t nb_block_H,uint8_t p,
		 uint8_t l,uint8_t *k) 
{
	for (uint8_t q = 0; q < nb_block_H; q++){
		for (uint8_t c = 0; c < _BWIDTH; c++){
			etire_pix(MCU_ds,MCU_us,h_factor,v_factor,
				  nb_block_H,p,l,q,c,*k);
			*k = *k + h_factor;
		}
	}
}

/* Etire les lignes sur toute la _BHEIGHT et sur le
 * nombre de blocs verticaux (nb_block_V) */
void upsampler(uint8_t *MCU_ds,uint8_t *MCU_us,uint8_t h_factor, 
	       uint8_t v_factor,uint16_t nb_block_H,uint16_t nb_block_V) 

{
	// Nombre de blocs initiaux avant sur-échantillonage
	uint16_t nb_block_H_init = nb_block_H/h_factor;
	uint16_t nb_block_V_init = nb_block_V/v_factor;
	uint8_t k = 0;
	// Longueur de ligne
	uint8_t longueur_ligne = nb_block_H_init * _BWIDTH * h_factor;
	for (uint8_t p = 0; p < nb_block_V_init; p++){
		for (uint8_t l = 0; l < _BHEIGHT; l++){
			etire_ligne(MCU_ds,MCU_us,h_factor,v_factor,
				    nb_block_H_init,p,l,&k);
			k = k + (v_factor - 1) * longueur_ligne;
		}
	}
}


/* PILOTE DE TEST */

/*int main(void)
{

	uint8_t nb_block_H = 4;
	uint8_t nb_block_V = 1;
	uint8_t h_factor = 2; 
	uint8_t v_factor = 1;
	uint32_t taille_init=_BSIZE*nb_block_H*nb_block_V/(h_factor*v_factor);
	uint8_t tab[taille_init];

	// construction du tableau

	for (uint32_t i = 0; i < taille_init; i++){
		tab[i] = i;
	}
	uint8_t * MCU_ds = &tab[0]; 

	// Affichage des blocs

	printf("\n Liste des blocs adjacents dans la MCU_ds : \n \n") ;
	for (uint32_t i = 0; i < taille_init; i++){
		printf("%i ",MCU_ds[i]);
		if (((i+1) % (_BWIDTH)) == 0){
			printf("\n");
		}
		if (((i+1) % (_BSIZE)) == 0){
			printf("\n");
		}
	}

	uint32_t new_size=taille_init*h_factor*v_factor;
	uint8_t tab_out[new_size];
	uint8_t * MCU_us = &tab_out[0];
	upsampler(MCU_ds,MCU_us,h_factor,v_factor,nb_block_H,nb_block_V);
	printf("tableau en sortie : \n \n") ;
	for (uint32_t i = 0; i < new_size; i++){
		printf("%4i",MCU_us[i]);
		if (((i+1) % (nb_block_H * _BWIDTH )) == 0){
			printf("\n");
		}
	}
	printf("\n");
}

*/

