
#include "turtle.h"

#include <string.h>
#include <stdlib.h>
#include <math.h>

#define PI 3.14159265358979323846

/* adauga poztia si orientarea actuala a testoasei in stiva de stari pe care
testoasa le traverseaza */
void add_state(turtle_states_t *t_state, turtle_t current_turtle)
{
	turtle_t *aux = realloc(t_state->turtle, (t_state->size + 1) *
								   sizeof(turtle_t));
	if (!aux) {
		fprintf(stderr, "realloc() failed\n");
		return;
	}
	t_state->turtle = aux;
	t_state->turtle[t_state->size] = current_turtle;
	t_state->size++;
}

/* extrage ultima stare a testoasei - care devine cea actuala si o elimina din
stiva de stari pe care testoasa le traverseaza */
int restore_state(turtle_states_t *t_state, turtle_t *removed_turtle)
{
	if (t_state->size == 0) {
		return 0;
	}

	t_state->size--;
	*removed_turtle = t_state->turtle[t_state->size];

	// eliberarea zonei de memorie a starii extrase
	if (t_state->size == 0) {
		free(t_state->turtle);
		t_state->turtle = NULL;
	} else {
		turtle_t *aux = realloc(t_state->turtle, t_state->size *
								  sizeof(turtle_t));
		if (!aux) {
			fprintf(stderr, "realloc() failed\n");
			return 0;
		}
		t_state->turtle = aux;
	}

	return 1;
}

/* implementarea algoritmului lui Bresenham */
void draw_line(image_t *img, int x0, int y0, int x1, int y1, unsigned char r,
			   unsigned char g, unsigned char b)
{
	int dx, dy, sx, sy, err;
	if (x1 > x0) {
		dx = x1 - x0;
		sx = 1;
	} else {
		dx = x0 - x1;
		sx = -1;
	}
	if (y1 > y0) {
		dy = y1 - y0;
		sy = 1;
	} else {
		dy = y0 - y1;
		sy = -1;
	}
	dy *= -1;
	err = dx + dy;

	do {
		draw_pixel(img, x0, y0, r, g, b);

		int e2 = 2 * err;
		if (e2 >= dy) {
			err += dy;
			x0 += sx;
		}
		if (e2 <= dx) {
			err += dx;
		y0 += sy;
		}
	} while (x0 != x1 || y0 != y1);

	draw_pixel(img, x0, y0, r, g, b);
}

/* realizeaza intreaga miscare a testoasei pe imagine conform sirului derivat
 din L-system si deseneaza liniile in culoarea indicata de combinatia canalelor
 RGB */
int turtle_movement(image_t *img, lsystem_t *lsys, double x, double y,
					double step, double angle, double delta, int n,
					unsigned char r, unsigned char g, unsigned char b)
{
	char *path = derive(lsys, n);
	if (!path) {
		return 0;
	}
	int len = strlen(path);

	turtle_t turtle;
	turtle.x = x;
	turtle.y = y;
	turtle.ang = angle;

	turtle_states_t t_state;
	t_state.turtle = NULL;
	t_state.size = 0;

	for (int i = 0; i < len; i++) {
		if (path[i] == 'F') {
			double rad = turtle.ang * PI / 180.0;

			double next_x = turtle.x + step * cos(rad);
			double next_y = turtle.y + step * sin(rad);

			int x0 = /*(int)*/lround(turtle.x);
			int y0 = /*(int)*/lround(turtle.y);
			int x1 = /*(int)*/lround(next_x);
			int y1 = /*(int)*/lround(next_y);

			if (x0 == x1 && y0 == y1) {
				draw_pixel(img, x0, y0, r, g, b);
			} else {
				draw_line(img, x0, y0, x1, y1, r, g, b);
			}
			turtle.x = next_x;
			turtle.y = next_y;
		} else if (path[i] == '+') {
			turtle.ang += delta;
		} else if (path[i] == '-') {
			turtle.ang -= delta;
		} else if (path[i] == '[') {
			add_state(&t_state, turtle);
		} else if (path[i] == ']') {
			turtle_t removed_turtle;
			int is_state = restore_state(&t_state, &removed_turtle);
			if (is_state) {
				turtle = removed_turtle;
			}
		}
	}
	free(t_state.turtle);
	free(path);
	return 1;
}
