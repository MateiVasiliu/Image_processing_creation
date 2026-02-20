
#include "lsystem.h"
#include "font.h"
#include "ppm_image.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* aloca bitmap-ul caracterului completand cu whitespace-ul necesar pana se
ajunge la un multiplu de 8 - un byte intreg*/
char **allocate_bitmap(int bbh, int needed_bytes)
{
	char **bitmap = (char **)malloc(bbh * sizeof(char *));
	if (!bitmap) {
		fprintf(stderr, "malloc() failed\n");
		return NULL;
	}
	for (int i = 0; i < bbh; i++) {
		bitmap[i] = (char *)calloc((needed_bytes * BYTE_LEN + 1),
								   sizeof(char));
		if (!bitmap[i]) {
			fprintf(stderr, "calloc() failed\n");
			for (int j = 0; j < i; j++) {
				free(bitmap[j]);
			}
			free(bitmap);
			return NULL;
		}
	}
	return bitmap;
}

/* transforma fiecare caracter din baza 16 in binar folosind operatii pe biti*/
char *hex_to_binary(char *line, int needed_bytes)
{
	char *binary_string = calloc(needed_bytes * 8 + 1, sizeof(char));
	if (!binary_string) {
		fprintf(stderr, "calloc() failed\n");
		return NULL;
	}

	int len = strlen(line), pos = 0;
	for (int i = 0; i < len; i++) {
		int value;

		if (line[i] >= '0' && line[i] <= '9') {
			value = line[i] - '0';
		} else if (line[i] >= 'A' && line[i] <= 'F') {
			value = 10 + (line[i] - 'A');
		}

		for (int j = 3; j >= 0; j--) {
			binary_string[pos++] = ((value >> j) & 1) + '0';
		}
	}
	binary_string[pos] = '\0';
	return binary_string;
}

/* construieste bitmap-ul caracterului ca un vector coloana de siruri binare */
void build_bitmap(char **bitmap, int bbh, int needed_bytes, FILE *bdf_file)
{
	for (int i = 0; i < bbh; i++) {
		char *line = read_text(bdf_file);
		if (!line) {
			return;
		}
		char *binary_line = hex_to_binary(line, needed_bytes);
		strcpy(bitmap[i], binary_line);

		free(line);
		free(binary_line);
	}
}

/*urmatoarele functii extrag valorile pentru fiecare atribut al fontului /
caracterului facand parsing pe linia corespunzatoare din fisierul .bdf */

/* extrage numarul de caractre al fontului si aloca un vector ce va contine
acele caractere si toate caracteristicile sale */
void chars_attribute(font_t *font, char *line)
{
	char *line_copy = copy_strdup(line + 6);
	if (!line_copy) {
		fprintf(stderr, "copy_strdup() failed\n");
		return;
	}
	char *p = strtok(line_copy, " ");
	font->nglyphs = atoi(p);
	free(line_copy);
	if (font->nglyphs > 0) {
		font->glyphs = (character_t *)calloc(font->nglyphs,
											 sizeof(character_t));
		if (!font->glyphs) {
			fprintf(stderr, "calloc() failed\n");
			return;
	}
	} else {
		font->glyphs = NULL;
	}
}

/* extrage codul caracterului - acelasi cu codul ASCII */
void encoding_attribute(character_t *current_char, char *line)
{
	if (current_char) {
		char *line_copy = copy_strdup(line + 8);
		if (!line_copy) {
			fprintf(stderr, "copy_strdup() failed\n");
			return;
		}
		char *p = strtok(line_copy, " ");
		current_char->code = atoi(p);
		free(line_copy);
	}
}

/* extrage valorile cu care se va muta cursorul pt fiecare caracter */
void dwidth_attribute(character_t *current_char, char *line)
{
	char *line_copy = copy_strdup(line + 6);
	if (!line_copy) {
		fprintf(stderr, "copy_strdup() failed\n");
		return;
	}
	if (current_char) {
		char *p = strtok(line_copy, " ");
		current_char->dwx = atoi(p);
		p = strtok(NULL, " ");
		current_char->dwy = atoi(p);
	}
	free(line_copy);
}

/* extrage cele 4 valori pentru bounding box - dimensiunile matricei de pixeli
si deplaarea pe axele Ox si Oy */
void bbx_attribute(character_t *current_char, char *line)
{
	char *line_copy = copy_strdup(line + 4);
	if (!line_copy) {
		fprintf(stderr, "copy_strdup() failed\n");
		return;
	}
	if (current_char) {
		char *p = strtok(line_copy, " ");
		current_char->bbw = atoi(p);
		p = strtok(NULL, " ");
		current_char->bbh = atoi(p);
		p = strtok(NULL, " ");
		current_char->bbxoff = atoi(p);
		p = strtok(NULL, " ");
		current_char->bbyoff = atoi(p);
	}
	free(line_copy);
}

/* incarca fontul in memoria programului extragand din fisierul bdf doar
atributele importante ale fontului si ale caracterelor continute */
font_t *load_font(char *file_name)
{
	FILE *bdf_file = fopen(file_name, "rt");
	if (!bdf_file) {
		fprintf(stderr, "Can't open file %s\n", file_name);
		return NULL;
	}
	font_t *font = (font_t *)calloc(1, sizeof(font_t));
	if (!font) {
		fprintf(stderr, "calloc() failed\n");
		return NULL;
	}

	character_t *current_char = NULL;
	int char_cnt = 0;
	char *line = read_text(bdf_file);
	while (line) {
		if (strncmp(line, "FONT ", 5) == 0) {
			font->name = copy_strdup(line + 5);
			if (!font->name) {
				fprintf(stderr, "copy_strdup() failed\n");
				return NULL;
			}
			font->name[strcspn(font->name, "\n")] = '\0';
		} else if (strncmp(line, "CHARS", 5) == 0) {
			chars_attribute(font, line);

		} else if (strncmp(line, "STARTCHAR", 9) == 0) {
			current_char = (character_t *)calloc(1, sizeof(character_t));
			if (!current_char) {
				fprintf(stderr, "calloc() failed\n");
				return NULL;
			}

		} else if (strncmp(line, "ENCODING", 8) == 0) {
			encoding_attribute(current_char, line);

		} else if (strncmp(line, "DWIDTH", 6) == 0) {
			dwidth_attribute(current_char, line);

		} else if (strncmp(line, "BBX", 3) == 0) {
			bbx_attribute(current_char, line);

		} else if (strcmp(line, "BITMAP") == 0) {
			if (current_char) {
				int needed_bytes = (current_char->bbw + 7) / 8;
				current_char->bitmap = allocate_bitmap(current_char->bbh,
													   needed_bytes);
				build_bitmap(current_char->bitmap, current_char->bbh,
							 needed_bytes, bdf_file);
			}

		} else if (strcmp(line, "ENDCHAR") == 0) {
			font->glyphs[char_cnt++] = *current_char;
			free(current_char);
			current_char = NULL;
		}
		free(line);
		line = read_text(bdf_file);
	}
	fclose(bdf_file);
	return font;
}

/* extrage caracterul din font impreuna cu toate caracteristicile lui,
identificandu-l pe baza codului sau ASCII */
character_t *find_glyph(font_t *font, char c)
{
	for (int i = 0; i < font->nglyphs; i++) {
		if (font->glyphs[i].code == (unsigned char)c) {
			return &font->glyphs[i];
		}
	}
	return NULL;
}

/* deseneaza textul dat pe imagine in fontul dat si in culoarea data de
combinatia RGB incepand de la coordonatele indicate */
int draw_text(image_t *img, font_t *font, char *text, int start_x,
			  int start_y, unsigned char r, unsigned char g, unsigned char b)
{
	if (!img) {
		return 0;
	}
	if (!font) {
		return 0;
	}
	int len = strlen(text);
	int cursor_x = start_x, cursor_y = start_y;
	for (int i = 0; i < len; i++) {
		character_t *character = find_glyph(font, text[i]);

		for (int l = 0; l < character->bbh; l++) {
			for (int c = 0; c < character->bbw; c++) {
				if (character->bitmap[l][c] == '1') {
					int x_coord = cursor_x + character->bbxoff + c;
					int y_coord = cursor_y + character->bbyoff +
								  (character->bbh - 1 - l);
					draw_pixel(img, x_coord, y_coord, r, g, b);
				}
			}
		}
		cursor_x += character->dwx;
		cursor_y += character->dwy;
	}

	return 1;
}
