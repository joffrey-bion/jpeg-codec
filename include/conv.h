#ifndef __CONV_H__
#define __CONV_H__

#include <stdint.h>
//#include <stdlib.h>

extern void YCbCr_to_ARGB(uint8_t *YCbCr_MCU[3], uint32_t *RGB_MCU,
			  uint32_t nb_block_H, uint32_t nb_block_V);

#endif

