
#ifndef TURTLE_H
#define TURTLE_H

#include "ppm_image.h"
#include "lsystem.h"

typedef struct {
	double x, y;
	double ang;
} turtle_t;

typedef struct {
	turtle_t *turtle;
	int size;
} turtle_states_t;

void add_state(turtle_states_t *t_state, turtle_t current_turtle);

int restore_state(turtle_states_t *t_state, turtle_t *removed_turtle);

void draw_line(image_t *img, int x0, int y0, int x1, int y1, unsigned char r,
			   unsigned char g, unsigned char b);

int turtle_movement(image_t *img, lsystem_t *lsys, double x, double y,
					double step, double angle, double delta, int n,
					unsigned char r, unsigned char g, unsigned char b);

#endif
