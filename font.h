
#ifndef FONT_H
#define FONT_H

#include <stdio.h>

#include "ppm_image.h"

#define BYTE_LEN 8

typedef struct {
	int code;
	int dwx, dwy;
	int bbw, bbh, bbxoff, bbyoff;
	char **bitmap;
} character_t;

typedef struct {
	char *name;
	int nglyphs;
	character_t *glyphs;
} font_t;

char **allocate_bitmap(int bbh, int needed_bytes);

int extract_number_from_string(char *line, int pos);

char *hex_to_binary(char *line, int needed_bytes);

void build_bitmap(char **bitmap, int bbh, int needed_bytes, FILE *bdf_file);

font_t *load_font(char *file_name);

int draw_text(image_t *img, font_t *font, char *text, int start_x,
			  int start_y, unsigned char r, unsigned char g, unsigned char b);

#endif
