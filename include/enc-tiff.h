#ifndef _ENC_TIFF_H
#define _ENC_TIFF_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <math.h>

#include "inout.h"

extern int read_tiff_file_header(FILE *tiff_file, uint16_t *im_height, 
				 uint16_t *im_width, uint8_t *MCU_height);
extern void get_tiff_MCU(FILE *tiff_file, uint32_t w, uint32_t *MCU);

#endif
