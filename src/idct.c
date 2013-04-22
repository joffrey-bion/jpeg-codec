#include "../include/idct.h"

uint8_t NormalisationF(float x) 
{
	if (x > 255) {
		x = 255;
	}
	else if (x < 0 ) {
		x = 0;
	}
	return (uint8_t)x;
}

float Coefficient_C(int32_t x) 
{ 
	if (x == 0) {
		return(M_SQRT1_2); 
	} else {
		return(1.);
	}
}

/* Application IDCT du sujet */
uint8_t S_x_y(int32_t x,int32_t y,int32_t *input ) 
{
	float S = 0;
	for (int i = 0; i < 8; i++) {
	 	for (int j = 0; j < 8; j++) {
			S = S + Coefficient_C(i) * Coefficient_C(j) *
			  cos(((1. + 2*x) * i * (M_PI)) / 16.) *
			  cos(((1. + 2*y) * j * (M_PI)) / 16.) *
			  (float)input[8*i + j];
		}
	}

	return NormalisationF((S/4.)+128.);
}


void IDCT(int32_t *input, uint8_t *output) 
{
	for (int i = 1; i <= 8; i++) {
 		for (int j = 1; j <= 8; j++) {
			output[8*(i-1)+j-1] = S_x_y(i,j,input);
		}
	}

}

#if 0

/* Application de l'IDCT III donnée par Vetterli 2D */
uint8_t S_x_y_dct(int32_t x,int32_t y,uint8_t *input ) 
{
	float S = 0;
	for (int i = 0; i < 8; i++) {
	 	for (int j = 0; j < 8; j++) {
			S = S + Coefficient_C(x) * Coefficient_C(y) *
			  cos(((1. + 2*i) * x * (M_PI)) / 16.) *
			  cos(((1. + 2*j) * y * (M_PI)) / 16.) *
			  (float)input[8*i + j];
		}
	}
	return NormalisationF((S/4.)+128.);
}

void DCT(uint8_t *input, int32_t *output) 
{
	for (int32_t i = 1; i <= 8; i++) {
 		for (int32_t j = 1; j <= 8; j++) {
			output[8*(i-1)+j-1] = S_x_y_dct(i,j,input);
		}
	}

}

#endif
