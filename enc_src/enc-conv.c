#include "../include/enc-conv.h"

uint8_t Rouge(uint32_t RGB)
{
        return (RGB & 0xff0000) >> 16;
}

uint8_t Vert(uint32_t RGB)
{
	return (RGB & 0xff00) >> 8;
}

uint8_t Bleu(uint32_t RGB)
{
        return (RGB & 0xff);
}

void ARGB_to_YCbCr(uint32_t *RGB_MCU,uint8_t  *YCbCr_MCU[3],
		uint32_t nb_block_H, uint32_t nb_block_V) 
{

	uint8_t R;
	uint8_t G;
	uint8_t B;

	uint8_t *Y1 = YCbCr_MCU[0]; 
	uint8_t *Cb1 = YCbCr_MCU[1];
	uint8_t *Cr1 = YCbCr_MCU[2];

	for (uint32_t i = 0; i < 8*nb_block_H*8*nb_block_V; i++) {

		R = Rouge(RGB_MCU[i]);
		G = Vert(RGB_MCU[i]);
		B = Bleu(RGB_MCU[i]);

		Y1[i]  =  0.299  * R + 0.587  * G + 0.114  * B;
		Cb1[i] = -0.1687 * R - 0.3313 * G + 0.5    * B + 128;
		Cr1[i] =  0.5    * R - 0.4187 * G - 0.0813 * B + 128;
	}
}


/*

Test à ajouter

*/
