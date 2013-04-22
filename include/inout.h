#ifndef __INOUT_H
#define __INOUT_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

#include "jpeg.h"

#define _DRY_RUN 0
/* 1 pour afficher les donnees brutes sans decoder */

#define _DISPLAY_HEAD 1
/* 1 pour afficher les marqueurs */
/* 2 pour afficher le contenu des segments de l'en-tete */

#define _DISPLAY_BODY 1
/* 1 pour afficher les grandes etapes du decodage */
/* 2 pour afficher les traces d'appels aux differents modules en detail */

#define _DISPLAY_HUFF 0
/* 1 pour afficher le detail lors de la creation des tables de huffman */

#define _DISPLAY_DECOD 0
/* 1 pour afficher le detail lors du decodage du RLE et DPCM */

#define _DISPLAY_BLOCK 0
/* 1 pour afficher le block frequentiel decode par unpack_block */

#define _DISPLAY_RGB_MCU 0
/* 1 pour afficher chaque MCU decodee en RGB */

#define printmark(x,...) if (_DISPLAY_HEAD >= 1) printf(x,##__VA_ARGS__);
#define printhead(x,...) if (_DISPLAY_HEAD == 2) printf(x,##__VA_ARGS__);
#define printbody(x,...) if (_DISPLAY_BODY>=1||_DRY_RUN) printf(x,##__VA_ARGS__);
#define printbody1(x,...) if (_DISPLAY_BODY == 1) printf(x,##__VA_ARGS__);
#define printbody2(x,...) if (_DISPLAY_BODY == 2) printf(x,##__VA_ARGS__);
#define printhuff(x,...) if (_DISPLAY_HUFF) printf(x,##__VA_ARGS__);
#define printdecod(x,...) if (_DISPLAY_DECOD) printf(x,##__VA_ARGS__);
#define printblock(x) if (_DISPLAY_BLOCK) print_freq_block(x);
#define printRGBMCU(x,y) if (_DISPLAY_RGB_MCU) print_px_MCU(x,y);

extern uint8_t read_byte(uint32_t *cpt, FILE *f);

extern uint16_t read_2bytes(uint32_t *cpt, FILE *f);

extern void skip_size(uint32_t size, uint32_t *cpt, FILE *f);

extern uint32_t read_block_size(uint32_t *cpt, FILE *f);

extern void display_data_and_EOI(uint32_t *cpt, FILE *f);

extern void print_freq_block(int32_t T[_BSIZE]);

extern void print_px_MCU(uint32_t *MCU, uint8_t MCU_size[2]);
#endif
