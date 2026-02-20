
#include "ppm_image.h"

#include <stdio.h>
#include <stdlib.h>

/* incarca in memoria programului imaginea in format ppm, extragand din fisier
dimensiunile si pixelii */
image_t *img_load(char *file_name)
{
	FILE *ppm_file = fopen(file_name, "rb");
	if (!ppm_file) {
		fprintf(stderr, "Can't open file %s\n", file_name);
		return NULL;
	}
	char magic_bytes[3] = {0};
	fread(magic_bytes, sizeof(char), 2, ppm_file);
	fgetc(ppm_file);
	int w, h, channel;

	fscanf(ppm_file, "%d %d", &w, &h);
	fscanf(ppm_file, "%d", &channel);
	fgetc(ppm_file);

	image_t *img = (image_t *)malloc(1 * sizeof(image_t));
	if (!img) {
		fprintf(stderr, "malloc() failed\n");
		fclose(ppm_file);
		return NULL;
	}

	img->w = w;
	img->h = h;
	img->px = (unsigned char *)calloc(3 * img->w * img->h,
									  sizeof(unsigned char));
	if (!img->px) {
		fprintf(stderr, "calloc() failed\n");
		free(img);
		fclose(ppm_file);
		return NULL;
	}
	fread(img->px, sizeof(unsigned char), 3 * img->w * img->h, ppm_file);

	fclose(ppm_file);
	return img;
}

// salveaza imaginea in format ppm ca un fisier binar
int save(char *file_name, image_t *img)
{
	FILE *ppm_file = fopen(file_name, "wb");
	if (!ppm_file) {
		fprintf(stderr, "Can't open file %s\n", file_name);
		return 0;
	}
	if (!img) {
		return 0;
	}

	fprintf(ppm_file, "P6\n%d %d\n255\n", img->w, img->h);
	fwrite(img->px, sizeof(unsigned char), 3 * img->w * img->h, ppm_file);
	fclose(ppm_file);
	return 1;
}

/* deseneaza pixelul in imagine la coordonatele date, in culoarea indicata de
combinatia valorilor RGB date, daca nu depaseste marginile */
void draw_pixel(image_t *img, int x0, int y0, unsigned char r, unsigned char g,
				unsigned char b)
{
	if ((x0 >= 0 && x0 < img->w) && (y0 >= 0 && y0 < img->h)) {
		int row = img->h - 1 - y0;
		int px_id = (row * img->w + x0) * 3;

		img->px[px_id] = r;
		img->px[px_id + 1] = g;
		img->px[px_id + 2] = b;
	}
}
