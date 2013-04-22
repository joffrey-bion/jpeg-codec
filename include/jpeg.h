/* Projet C - Sujet JPEG */
#ifndef __JPEG_H__
#define __JPEG_H__

#include <stdint.h>

#define _BWIDTH 8
#define _BHEIGHT 8
#define _BSIZE ((_BWIDTH)*(_BHEIGHT))

typedef struct _huff_table_t {
	uint16_t code;
	int8_t  value;
	int8_t is_elt;
	struct _huff_table_t *parent;
	struct _huff_table_t *left;
	struct _huff_table_t *right;
} huff_table_t;

typedef struct _scan_desc_t {
	uint8_t bit_count;
	uint8_t window;
	int32_t pred[3];
	huff_table_t *table[2][3];
} scan_desc_t;


#endif
