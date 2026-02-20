
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "ppm_image.h"
#include "lsystem.h"
#include "turtle.h"
#include "font.h"
#include "undo_redo.h"

#define TEMP_BUFF 101
#define BITS_IN_PIXEL 24

/* formeaza numarul real din sirul de carcatere */
double double_from_string(char *arg)
{
	double nr = 0.0;
	int sign = 1, i = 0;

	if (arg[i] == '-') {
		sign = -1;
		i++;
	}

	while (arg[i] != '\0' && arg[i] != '.') {
		nr = nr * 10 + (arg[i] - '0');
		i++;
	}

	if (arg[i] == '.') {
		i++;
		double p = 0.1;
		while (arg[i] != '\0') {
			nr += (arg[i] - '0') * p;
			p *= 0.1;
			i++;
		}
	}

	return sign * nr;
}

/* extrage numele fisierului ce contine imaginea in format ppm si adauga
 comanda in stiva de comenzi undoable, obtinandu-se imaginea curenta */
void check_load(/*char *attributes,*/ stack_t *undo_stack, stack_t *redo_stack,
				state_t *current_state)
{
	char *file_name = strtok(NULL, " ");
	if (!file_name) {
		return;
	}
	image_t *img = img_load(file_name);
	if (!img) {
		printf("Failed to load %s\n", file_name);
	} else {
		stack_add(undo_stack, current_state, "LOAD", file_name);
		free_stack(redo_stack);

		image_t *aux_old = current_state->img;
		current_state->img = img;
		free_image(aux_old);

		printf("Loaded %s (PPM image %dx%d)\n", file_name, img->w, img->h);
	}
}

/* salveaza imaginea daca aceasta exista in memoria programului*/
void check_save(/*char *attributes,*/ state_t *current_state)
{
	char *file_name = strtok(NULL, " ");
	if (!current_state->img) {
		printf("No image loaded\n");
	} else {
		save(file_name, current_state->img);
		printf("Saved %s\n", file_name);
	}
}

/* extrage numele fisierului din comanda introdusa si incarca in memoria
 programului L-system-ul dat si in stiva de comenzi undoable */
void check_lsystem(/*char attributes,*/ stack_t *undo_stack,
				   stack_t *redo_stack, state_t *current_state)
{
	char *file_name = strtok(NULL, " ");
	if (!file_name) {
		return;
	}
	lsystem_t *lsys = lsys_load(file_name);
	if (!lsys) {
		printf("Failed to load %s\n", file_name);
	} else {
		stack_add(undo_stack, current_state, "LSYSTEM", file_name);
		free_stack(redo_stack);

		lsystem_t *aux_old = current_state->lsys;
		current_state->lsys = lsys;
		free_lsystem(aux_old);

		printf("Loaded %s (L-system with %d rules)\n", file_name, lsys->nrules);
	}
}

/* verifica daca exista un L-system incarcat si aplica derivarea ceruta,
 extrgand numarul n - ordinul derivarii */
void check_derive(/*char *attributes,*/ state_t *current_state)
{
	if (!current_state->lsys) {
		printf("No L-system loaded\n");
	} else {
		char *arg = strtok(NULL, " ");
		int n = atoi(arg);
		char *n_th_deriv = derive(current_state->lsys, n);
		if (n_th_deriv) {
			printf("%s\n", n_th_deriv);
			free(n_th_deriv);
		}
	}
}

/* verifica daca exista o imagine si un L-system incarcate si apoi extrage pe
 rand toate atributele comenzii - coordonatele testoasei, pasul, unghiul de
 orientare, pasul unghiular, ordinul derivarii si valorile canalelor RGB, iar
 apoi este incarcat in stiva de undo si se realizeaza desenul*/
void check_turtle(/*char *attributes,*/ stack_t *undo_stack, stack_t
				  *redo_stack, state_t *current_state)
{
	if (!current_state->img) {
		printf("No image loaded\n");
	} else if (!current_state->lsys) {
		printf("No L-system loaded\n");
	} else {

		char *arg = strtok(NULL, " ");
		if (!arg) {
			return;
		}
		double x = double_from_string(arg);

		arg = strtok(NULL, " ");
		if (!arg) {
			return;
		}
		double y = double_from_string(arg);

		arg = strtok(NULL, " ");
		if (!arg) {
			return;
		}
		double step = double_from_string(arg);

		arg = strtok(NULL, " ");
		if (!arg) {
			return;
		}
		double angle = double_from_string(arg);

		arg = strtok(NULL, " ");
		if (!arg) {
			return;
		}
		double delta = double_from_string(arg);

		arg = strtok(NULL, " ");
		if (!arg) {
			return;
		}
		int n = atoi(arg);

		arg = strtok(NULL, " ");
		if (!arg) {
			return;
		}

		unsigned char r = atoi(arg);
		arg = strtok(NULL, " ");
		if (!arg) {
			return;
		}

		unsigned char g = atoi(arg);
		arg = strtok(NULL, " ");
		if (!arg) {
			return;
		}

		unsigned char b = atoi(arg);

		stack_add(undo_stack, current_state, "TURTLE", NULL);
		free_stack(redo_stack);

		turtle_movement(current_state->img, current_state->lsys, x, y, step,
						angle, delta, n, r, g, b);

		printf("Drawing done\n");
	}
}

/* verifica daca se incarca fontul dat, incarcandu-l si in stiva de undo */
void check_font(/*char *attributes,*/ stack_t *undo_stack, stack_t *redo_stack,
				state_t *current_state)
{
	char *file_name = strtok(NULL, " ");
	if (!file_name) {
		return;
	}
	font_t *font = load_font(file_name);
	if (!font) {
		printf("Failed to load %s\n", file_name);
	} else {
		stack_add(undo_stack, current_state, "FONT", file_name);
		free_stack(redo_stack);
		free_font(current_state->font);
		current_state->font = font;

		printf("Loaded %s (bitmap font %s)\n", file_name, font->name);
	}
}

/* verifca daca exista o imagine si un font introduse si apoi extrage
 atributele comenzii - textul de scris, coordonatele de inceput si culoarea
 data de valorile RGB ,desenand apoi textul si incarcand in stiva de undo
 comanda*/
void check_type(/*char *attributes, */stack_t *undo_stack, stack_t *redo_stack,
				state_t *current_state)
{
	if (!current_state->img) {
		printf("No image loaded\n");
	} else if (!current_state->font) {
		printf("No font loaded\n");
	} else {
		stack_add(undo_stack, current_state, "TYPE", NULL);
		free_stack(redo_stack);

		char *text = strtok(NULL, "\"");
		if (!text) {
			text = "";
		}

		char *arg = strtok(NULL, " ");
		if (!arg) {
			return;
		}
		int start_x = atoi(arg);
		arg = strtok(NULL, " ");
		if (!arg) {
			return;
		}
		int start_y = atoi(arg);
		arg = strtok(NULL, " ");
		if (!arg) {
			return;
		}
		unsigned char r = atoi(arg);
		arg = strtok(NULL, " ");
		if (!arg) {
			return;
		}
		unsigned char g = atoi(arg);
		arg = strtok(NULL, " ");
		if (!arg) {
			return;
		}
		unsigned char b = atoi(arg);

		draw_text(current_state->img, current_state->font, text, start_x,
				  start_y, r, g, b);
		printf("Text written\n");
	}
}

/*extrage bitul de la pozitia data*/
int extract_bit(image_t *img, int pos)
{
	// caut canalul (echivalent cu un byte)
	int channel = pos / 8;
	int bit  = 7 - (pos % 8);

	return (img->px[channel] >> bit) & 1;
}

/* schimba valoarea bitului de la pozitia data din 0 in 1 si invers*/
void change_bit(image_t *img, int pos, int value)
{
	int channel = pos / 8;
	int bit  = 7 - (pos % 8);

	if (value == 1)
		img->px[channel] |= (1 << bit);
	else
		img->px[channel] &= ~(1 << bit);
}

/* realizeaza verificarea pe stringul de biti ce formeaza imaginea luand pe
 rand secvente de 4 biti si verificand daca sunt cele cerute cu problema,
modificandu-le si afisand mesajul coerespunzator */
int check_pixel_string(image_t *img)
{
	int total_bits = img->w * img->h * BITS_IN_PIXEL;

	for (int i = 0; i < total_bits - 3; i++) {

		int bit_sequence = 0;
		for (int j = 3; j >= 0; j--) {
			bit_sequence |= extract_bit(img, i + (3 - j)) << j;
		}

		if (bit_sequence == 2 || bit_sequence == 13) { // 0010 sau 1101

			int problem_bit_pos = i + 2;

			int previous_bit = extract_bit(img, problem_bit_pos);
			change_bit(img, problem_bit_pos, !previous_bit);

			int channel = problem_bit_pos / 8;
			int pixel = channel / 3;

			int x = pixel % img->w;
			int y = img->h - 1 - (pixel / img->w);

			unsigned char r = img->px[pixel * 3];
			unsigned char g = img->px[pixel * 3 + 1];
			unsigned char b = img->px[pixel * 3 + 2];

			printf("Warning: pixel at (%d, %d) may be read as (%d, %d, %d)\n",
				   x, y, r, g, b);

			change_bit(img, problem_bit_pos, previous_bit);
		}
	}

	return 1;
}

int main(void)
{
	state_t current_state;
	current_state.img = NULL;
	current_state.font = NULL;
	current_state.lsys = NULL;

	stack_t undo_stack, redo_stack;
	undo_stack.state = NULL;
	undo_stack.size = 0;
	redo_stack.state = NULL;
	redo_stack.size = 0;

	int done_reading = 0;
	while (!done_reading) {
		char *line = read_text(stdin);
		if (!line) {
			break;
		}
		char *copy_line = copy_strdup(line);
		if (!copy_line) {
			fprintf(stderr, "copy_strdup() failed\n");
			free(line);
			break;
		}

		char *command = strtok(copy_line, " ");
		if (!command) {
			free(line);
			free(copy_line);
			continue;
		}

		/* urmatoarele prelucrari le fac pe copy_line - sirul din care au ramas
		numai atributele, odata ce am verificat si separat comanda */
		if (strcmp(command, "LOAD") == 0) {
			check_load(&undo_stack, &redo_stack, &current_state);
		} else if (strcmp(command, "SAVE") == 0) {
			check_save(&current_state);
		} else if (strcmp(command, "UNDO") == 0) {
			int to_undo = undo(&undo_stack, &redo_stack, &current_state);
			if (to_undo == 0) {
				printf("Nothing to undo\n");
			}
		} else if (strcmp(command, "REDO") == 0) {
			int to_redo = redo(&undo_stack, &redo_stack, &current_state);
			if (to_redo == 0) {
				printf("Nothing to redo\n");
			}
		} else if (strcmp(command, "LSYSTEM") == 0) {
			check_lsystem(&undo_stack, &redo_stack, &current_state);
		} else if (strcmp(command, "DERIVE") == 0) {
			check_derive(&current_state);
		} else if (strcmp(command, "TURTLE") == 0) {
			check_turtle(&undo_stack, &redo_stack,
						 &current_state);
		} else if (strcmp(command, "FONT") == 0) {
			check_font(&undo_stack, &redo_stack, &current_state);
		} else if (strcmp(command, "TYPE") == 0) {
			check_type(&undo_stack, &redo_stack, &current_state);
		} else if (strcmp(command, "BITCHECK") == 0) {
			if (!current_state.img) {
				printf("No image loaded\n");
			} else {
				check_pixel_string(current_state.img);
			}
		} else if (strcmp(command, "EXIT") == 0) {
			done_reading = 1;
		}

		free(line);
		free(copy_line);
	}

	free_stack(&undo_stack);
	free_stack(&redo_stack);
	free_state(&current_state);
	return 0;
}
