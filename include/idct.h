/* Projet C - Sujet JPEG */
#ifndef __IDCT_H
#define __IDCT_H

#include <stdio.h>
#include <stdint.h>
//#include <stdlib.h>
#include <assert.h>
#include <math.h>

# define M_PI		3.14159265358979323846	// pi
# define M_SQRT1_2	0.70710678118654752440	// 1/sqrt(2)

extern void IDCT(int32_t *input, uint8_t *output);

#endif
