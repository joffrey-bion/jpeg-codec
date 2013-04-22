/* Projet C - Sujet JPEG */
#ifndef __UNPACK_BLOCK_H
#define __UNPACK_BLOCK_H

#include <stdint.h>
#include <assert.h>
#include "jpeg.h"
#include "inout.h"

extern void unpack_block(FILE *movie, scan_desc_t *scan_desc,
		uint32_t index, int32_t T[64]);


#endif
