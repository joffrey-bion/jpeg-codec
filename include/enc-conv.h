#ifndef __ENC_CONV_H__
#define __ENC_CONV_H__

#include <stdint.h>
#include <stdlib.h>

extern void ARGB_to_YCbCr(uint32_t *RGB_MCU, uint8_t  *YCbCr_MCU[3],
			  uint32_t nb_block_H, uint32_t nb_block_V);

#endif
