
#ifndef PPM_IMAGE_H
#define PPM_IMAGE_H

typedef struct {
	int w, h;
	unsigned char *px;
} image_t;

image_t *img_load(char *file_name);

int save(char *file_name, image_t *img);

void draw_pixel(image_t *img, int x0, int y0, unsigned char r, unsigned char g,
				unsigned char b);

#endif
