#include "../include/iqzz.h"
#define DEMI_TOUR_B diag_up = 0; i++; j--;
#define DEMI_TOUR_H diag_up = 1; i--; j++;
#define ACTION(ind,cond1,cond2) if (ind%2){ cond1 } else { cond2 }

/* Note sur les 3 définitions ci-dessus :
 * DEMI_TOUR_B effectue un demi-tour vers le bas
 * DEMI_TOUR_H effectue un demi-tour vers le haut
 * ACTION réalise la condition 1 ou 2 selon la parité de ind */

void iqzz_block(int32_t in[64], int32_t out[64], uint8_t table[64])
{
	int32_t i = 0;
	int32_t j = 0;
        int32_t k = 0;
	int32_t diag_up = 0;

	while (i < 8 && j < 8){
                out[i*8+j] = in[k] * table[k];
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

/*
int main(void){
	int32_t in[64] = {78, 45, -52, 20, -22, -7, -1, 16, -2, 4, -2, 12, -11, 0, 0, 0, 0, 4, 2, -5, -2, 2, 0, 2, -4, 0, 0, 0, 0, 0, 0, 0, 2, 2, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	int32_t out[64];
        uint8_t table[64];
	for (int i = 0; i<64; i++){
		table[i] = 1;
	}
	iqqz_block(in,out,table);
	for (int i = 0; i<8; i++){
		for (int j = 0; j<8; j++){
			printf("%i ",out[i*8+j]);
		}
		printf("\n");
	}
	
	return 0;
}
*/
