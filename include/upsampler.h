/* Projet C - Sujet JPEG */
#ifndef UPSAMPLER_H
#define UPSAMPLER_H

#include <stdio.h>
#include <stdint.h>
#include "jpeg.h"

extern void upsampler(uint8_t *MCU_ds, uint8_t *MCU_us,
		uint8_t h_factor, uint8_t v_factor,
		uint16_t nb_block_H, uint16_t nb_block_V);

#endif
