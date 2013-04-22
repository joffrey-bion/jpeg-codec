#include "../include/skip_segment.h"


void skip_segment(uint32_t *cpt, FILE *movie)
{
	uint16_t size = read_block_size(cpt,movie);
	skip_size(size-2,cpt,movie);
	uint8_t oct = read_byte(cpt,movie);
	assert(oct == 0xff);
}


/*
void skip_segment(uint32_t *cpt, FILE *movie)
{
	uint32_t oct = read_byte(cpt,movie);
	while (oct != 0xff) {
		oct = read_byte(cpt,movie);
	}
	assert(oct == 0xff);
}
*/

/*
void skip_segment(FILE *movie)
{
	uint32_t cpt = 0;
	uint32_t oct = read_byte(&cpt,movie);
	while (oct != 0xff) {
		oct = read_byte(&cpt,movie);
	}
}
*/
