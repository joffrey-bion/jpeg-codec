/* Projet C - Sujet JPEG */
#ifndef __TIFF_H__
#define __TIFF_H__

#include<stdint.h>
#include<stdio.h>

extern int init_tiff_file ( FILE * tifffile , uint32_t width ,
                                uint32_t length , uint32_t MCU_height ) ;

extern void write_tiff_file ( FILE * tifffile , uint32_t w, uint32_t h, 
			      uint32_t * image ) ;

extern int close_tiff_file(FILE *tifffile);

#endif
