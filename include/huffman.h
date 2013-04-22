/* Projet C - Sujet JPEG */
#ifndef __HUFFMAN_H
#define __HUFFMAN_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

#include "jpeg.h"
#include "inout.h"

extern void free_huffman_tables(huff_table_t *root);

extern int load_huffman_table(uint32_t *cpt, FILE *movie, huff_table_t *ht);

#endif
