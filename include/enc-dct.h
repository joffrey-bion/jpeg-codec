#ifndef _ENC_DCT_H
#define _ENC_DCT_H

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>
//#include "idct.h"

#define M_PI	      3.14159265358979323846	/* pi */
#define M_SQRT1_2     0.70710678118654752440	/* 1/sqrt(2) */
#define _COMPARE_DFT_ 0
#define print_comp(x,...) if (_COMPARE_DFT_) printf(x,##__VA_ARGS__);

extern void DCT_vetterli(uint8_t *block_in, int32_t *block_out);

#endif
