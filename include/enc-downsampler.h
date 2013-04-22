#ifndef _ENC_DOWNSAMPLER_H
#define _ENC_DOWNSAMPLER_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

#include "jpeg.h"

extern void downsampler(uint8_t *MCU_in, uint8_t *MCU_out, uint8_t h_factor, 
			uint8_t v_factor, uint16_t nb_block_H, 
			uint16_t nb_block_V);

#endif
