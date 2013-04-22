#include "../include/enc-qzz.h"

void qzz_block(int32_t in[64], int32_t out[64], uint8_t table[64], 
	       uint8_t qualite)
{
	int32_t i = 0;
	int32_t j = 0;
        int32_t k = 0;
	int32_t diag_up = 0;

	uint8_t quantif[64];
	uint8_t pos;

	while (i < 8 && j < 8){
		pos = i*8+j;
		quantif[pos] = 1+(i+j+1)*qualite;
		out[k] = in[pos];
		table[k] = quantif[pos];
                // ACTION à réaliser lorsque l'on touche un bord
		if (i == 0){
			ACTION(j,DEMI_TOUR_B,j++;)
		} else if (i == 7){
			ACTION(j,DEMI_TOUR_H,j++;)
		} else if (j == 0){
			ACTION(i,i++;,DEMI_TOUR_H)
		} else if (j == 7){
			ACTION(i,i++;,DEMI_TOUR_B)
		} else if (diag_up == 1){
                        // diagonale montante
			i--;
			j++;
		} else if (diag_up == 0){
                        // diagonale descendante
			i++;
			j--;
		}
		k++;
	}
}


#if 0
int main(void){
	int32_t in[64] = {4, 120, 100, 13, 16, 19, 22, 25, 120, 100, 13, 16, 19,
			  22, 25, 28, 100, 13, 16, 19, 22, 25, 28, 31, 13, 16, 
			  19, 22, 25, 28, 31, 34, 16, 19, 22, 25, 28, 31, 34,
                          37, 19, 22, 25, 28, 31, 34, 37, 40, 22, 25, 28, 31,
                          34, 37, 40, 43, 25, 28, 31, 34, 37, 40, 43, 46};
	int32_t out[64];
        uint8_t table[64];

	qzz_block(in,out,table,3);
	for (int i = 0; i<64; i++){
			printf("%i ",out[i]);
	}
	printf("\n");
	
	for (int i = 0; i<64; i++){
		printf("%i ",table[i]);
	}
	return 0;
}
#endif
