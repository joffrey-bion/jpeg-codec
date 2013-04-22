#ifndef _ENC_QZZ_H
#define _ENC_QZZ_H

#include <stdint.h>
#include <stdlib.h>

#define DEMI_TOUR_B diag_up = 0; i++; j--;
#define DEMI_TOUR_H diag_up = 1; i--; j++;
#define ACTION(ind,cond1,cond2) if (ind%2){ cond1 } else { cond2 }

/* 
 * Note sur les 3 définitions ci-dessus :
 * DEMI_TOUR_B effectue un demi-tour vers le bas
 * DEMI_TOUR_H effectue un demi-tour vers le haut
 * ACTION réalise la condition 1 ou 2 selon la parité de ind 
 */

extern void qzz_block(int32_t in[64], int32_t out[64], uint8_t table[64], 
		      uint8_t qualite);

#endif
