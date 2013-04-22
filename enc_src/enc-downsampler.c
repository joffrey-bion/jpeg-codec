#include "../include/enc-downsampler.h"

/* Sous-échantillonne les blocs ( <= 4 ) en entrée MCU_ds
 * et renvoie les blocs fusionnés dans MCU_us */
void downsampler(uint8_t *MCU_ds,uint8_t *MCU_us,uint8_t h_factor, 
	       uint8_t v_factor,uint16_t nb_block_H,uint16_t nb_block_V) 

{
	if ((h_factor == 1) && (v_factor == 1)){
                // pas de sous-échantillonage (4:4:4), on recopie les blocs
		uint32_t offset = nb_block_H * _BWIDTH;
		uint32_t k = 0;
		for (uint8_t p = 0; p < nb_block_V; p++){
			for (uint8_t q = 0; q < nb_block_H; q++){
				for (uint32_t i = 0; i < _BHEIGHT; i++) {
					for (uint32_t j = 0; j < _BWIDTH; j++) {
						MCU_us[p*_BSIZE*nb_block_H+q*_BSIZE+i*_BWIDTH+j] = MCU_ds[p*_BSIZE*nb_block_H+q*_BWIDTH+i*offset+j];
						k++;
					}
				}
			}
		}
	} else if ((h_factor == 2) & (v_factor == 1)){
                // sous-échantillonage 4:2:2
		assert((nb_block_H == 2) && (nb_block_V <= 2));
		uint32_t k = 0;
		for (uint8_t p = 0; p < nb_block_V; p++){
			for (uint8_t i = 0; i < _BHEIGHT; i++){
				for (uint8_t j = 0; j < _BWIDTH; j++){
                                        // coefficients courant du bloc 1 et 2
					uint8_t cb1 = MCU_ds[k];
					uint8_t cb2 = MCU_ds[k+1];
					MCU_us[i*_BWIDTH+j+64*p] = (cb1+cb2)/2.;
					k += 2;
				}
			}
		}
	} else if ((h_factor == 2) && (v_factor == 2)){
                // sous-échantillonage 4:2:0
		uint32_t offset = nb_block_H * _BWIDTH;
		assert((nb_block_H == 2) && (nb_block_V == 2));
		uint32_t k = 0;
		for (uint8_t i = 0; i < 8; i++){
			for (uint8_t j = 0; j < 8; j++){
				uint8_t cb1 = MCU_ds[k];
				uint8_t cb2 = MCU_ds[k+1];
				uint8_t cb3 = MCU_ds[k+offset];
				uint8_t cb4 = MCU_ds[k+offset+1];
				MCU_us[i*_BWIDTH + j] = (cb1+cb2+cb3+cb4)/4.;
				k += 2 + offset * ((k+2) % offset == 0);
			}
		}
	} else {
		printf("Erreur : paramètres d'échantillonage invalides\n");
	}
}


#if 0

int main(void)
{

	uint8_t nb_block_H = 2;
	uint8_t nb_block_V = 2;
	uint8_t h_factor = 1; 
	uint8_t v_factor = 1;
	uint32_t taille_bloc=64;
	uint32_t taille_init = nb_block_H*nb_block_V*taille_bloc;

	// construction du tableau
	uint8_t * MCU_ds = calloc(taille_init,sizeof(uint8_t));
       
	for (uint32_t i = 0; i < taille_bloc; i++){
		MCU_ds[i] = 10;
	}
	for (uint32_t i = 0; i < taille_bloc; i++){
		MCU_ds[i+64] = i;
	}
	for (uint32_t i = 0; i < taille_bloc; i++){
		MCU_ds[i+128] = 30;
	}
	for (uint32_t i = 0; i < taille_bloc; i++){
		MCU_ds[i+192] = 40;
	}

	// Affichage des blocs

	printf("\n Liste des blocs adjacents dans la MCU_ds : \n \n") ;
	for (uint32_t i = 0; i < taille_init; i++){
		printf("%2i ",MCU_ds[i]);
		if (((i+1) % (nb_block_H*_BWIDTH)) == 0){
			printf("\n");
		}
	}
	printf("\n");
	uint32_t new_size=(taille_init)/(h_factor*v_factor);
	uint8_t * MCU_us = calloc(new_size,sizeof(uint8_t)); 
	downsampler(MCU_ds,MCU_us,h_factor,v_factor,nb_block_H,nb_block_V);

	printf("tableau en sortie (de taille %u) : \n \n",new_size) ;

	for (uint32_t i = 0; i < new_size; i++){
		printf("%i ",MCU_us[i]);
		if (((i+1) % 8) == 0){
			printf("\n");
		}
		if (((i+1) % (taille_bloc)) == 0){
			printf("\n");
		}
	}

}

#endif
