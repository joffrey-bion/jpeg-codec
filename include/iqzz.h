/* Projet C - Sujet JPEG */
#ifndef __IQZZ_H
#define __IQZZ_H

#include <stdio.h>
#include <stdint.h>

extern void iqzz_block( int32_t in[64], int32_t out[64], uint8_t table[64]);

#endif
