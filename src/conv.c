#include "../include/conv.h"

uint8_t Normalisation(int32_t x) 
{
	if (x > 255) {
		x = 255;
	}
	else if (x < 0 ) {
		x = 0;
	}
	return (uint8_t)x;
}


void YCbCr_to_ARGB(uint8_t  *YCbCr_MCU[3], uint32_t *RGB_MCU,
		uint32_t nb_block_H, uint32_t nb_block_V) 
{
	/* On récupère les trois tableaux associés aux trois composantes */
	uint8_t *Y1 = YCbCr_MCU[0]; 
	uint8_t *Cb1 = YCbCr_MCU[1];
	uint8_t *Cr1 = YCbCr_MCU[2];
	uint32_t R, G, B;


	for (uint32_t i = 0; i < 8*nb_block_H*8*nb_block_V; i++) {

		R=Y1[i]-0.0009267*(Cb1[i]-128)+1.4016868*(Cr1[i]-128);
		G=Y1[i]-0.3436954*(Cb1[i]-128)-0.7141690*(Cr1[i]-128);
		B=Y1[i]+1.7721604*(Cb1[i]-128)+0.0009902*(Cr1[i]-128);

		R=Normalisation(R);
		G=Normalisation(G);
		B=Normalisation(B);

		/* 
		 * Formules simplifiées :
		 * R=Y1[i]+1.402*(Cr1[i]-128);
		 * G=Y1[i]-0.34414*(Cb1[i]-128)-0.71414*(Cr1[i]-128);
		 * B=Y1[i]+1.772*(Cb1[i]-128);
		 */

		R=R<<16;
		G=G<<8;
		RGB_MCU[i]=R+G+B;

	}

}


