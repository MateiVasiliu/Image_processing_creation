
#include "undo_redo.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* urmatoarele functii elibereaza fiecare componenta aflata in stiva ce
reprezinta memoria programului, inclusiv stiva */
void free_glyph(character_t *glyph)
{
	for (int i = 0; i < glyph->bbh; i++) {
		free(glyph->bitmap[i]);
	}
	free(glyph->bitmap);
	glyph->bitmap = NULL;
}

void free_image(image_t *img)
{
	if (img) {
		free(img->px);
		img->px = NULL;
		free(img);
	}
}

void free_font(font_t *font)
{
	if (font) {
		free(font->name);

		for (int i = 0; i < font->nglyphs; i++) {
			free_glyph(&font->glyphs[i]);
		}
		free(font->glyphs);
		free(font);
	}
}

void free_lsystem(lsystem_t *lsys)
{
	if (lsys) {
		free(lsys->axiom);
		lsys->axiom = NULL;

		for (int i = 0; i < MAX_CHAR; i++) {
			free(lsys->rules[i]);
			lsys->rules[i] = NULL;
		}
		free(lsys);
	}
}

void free_state(state_t *state)
{
	if (state->img) {
		free_image(state->img);
		state->img = NULL;
	}
	if (state->font) {
		free_font(state->font);
		state->font = NULL;
	}
	if (state->lsys) {
		free_lsystem(state->lsys);
		state->lsys = NULL;
	}
}

void free_stack(stack_t *stack)
{
	for (int i = 0; i < stack->size; i++) {
		free_state(&stack->state[i].state);
		free(stack->state[i].command);
		free(stack->state[i].file_name);
	}
	free(stack->state);
	stack->state = NULL;
	stack->size = 0;
}

/* urmatoarele functii realizeaza copierea prin deep copy a tuturor
componentelor programului pentru a le */
image_t *image_deep_copy(image_t *img)
{
	image_t *img_copy = (image_t *)calloc(1, sizeof(image_t));
	if (!img_copy) {
		fprintf(stderr, "calloc() failed\n");
		return NULL;
	}

	img_copy->h = img->h;
	img_copy->w = img->w;

	int nr_pixels = 3 * img_copy->h * img_copy->w;
	img_copy->px = (unsigned char *)calloc(nr_pixels, sizeof(unsigned char));
	if (!img_copy->px) {
		fprintf(stderr, "calloc() failed\n");
		free(img_copy);
		return NULL;
	}

	for (int i = 0; i < nr_pixels; i++) {
		img_copy->px[i] = img->px[i];
	}

	return img_copy;
}

int character_deep_copy(character_t *character_copy, character_t *character)
{
	character_copy->code = character->code;
	character_copy->dwx = character->dwx;
	character_copy->dwy = character->dwy;
	character_copy->bbh = character->bbh;
	character_copy->bbw = character->bbw;
	character_copy->bbxoff = character->bbxoff;
	character_copy->bbyoff = character->bbyoff;

	character_copy->bitmap = (char **)calloc(character->bbh, sizeof(char *));
	if (!character_copy->bitmap) {
		fprintf(stderr, "calloc() failed\n");
		return 0;
	}
	for (int i = 0; i < character->bbh; i++) {
		character_copy->bitmap[i] = copy_strdup(character->bitmap[i]);
		if (!character_copy->bitmap[i]) {
			fprintf(stderr, "copy_strdup() failed\n");
			for (int j = 0; j < i; j++) {
				free(character_copy->bitmap[j]);
			}
			free(character_copy->bitmap);
			return 0;
		}
	}

	return 1;
}

font_t *font_deep_copy(font_t *font)
{
	font_t *font_copy = (font_t *)calloc(1, sizeof(font_t));
	if (!font_copy) {
		fprintf(stderr, "calloc() failed\n");
		return NULL;
	}

	font_copy->name = NULL;
	font_copy->glyphs = NULL;
	font_copy->nglyphs = font->nglyphs;

	font_copy->name = copy_strdup(font->name);
	if (!font_copy->name) {
		fprintf(stderr, "copy_strdup() failed\n");
		free(font_copy);
		return NULL;
	}

	font_copy->glyphs = (character_t *)calloc(font_copy->nglyphs,
											  sizeof(character_t));
	if (!font_copy->glyphs) {
		fprintf(stderr, "calloc() failed\n");
		free(font_copy->name);
		free(font_copy);
		return NULL;
	}
	for (int i = 0; i < font_copy->nglyphs; i++) {
		int ok = character_deep_copy(&font_copy->glyphs[i], &font->glyphs[i]);
		if (ok == 0) {
			for (int j = 0; j < i; j++) {
				free_glyph(&font_copy->glyphs[j]);
			}
			free(font_copy->glyphs);
			free(font_copy->name);
			free(font_copy);
			return NULL;
		}
	}
	return font_copy;
}

lsystem_t *lsystem_deep_copy(lsystem_t *lsys)
{
	lsystem_t *lsys_copy = (lsystem_t *)calloc(1, sizeof(lsystem_t));
	if (!lsys_copy) {
		fprintf(stderr, "calloc() failed\n");
		return NULL;
	}

	lsys_copy->nrules = lsys->nrules;

	lsys_copy->axiom = copy_strdup(lsys->axiom);
	if (!lsys_copy->axiom) {
		fprintf(stderr, "copy_strdup() failed\n");
		free(lsys_copy);
		return NULL;
	}

	for (int i = 0; i < MAX_CHAR; i++) {
		if (lsys->rules[i]) {
			lsys_copy->rules[i] = copy_strdup(lsys->rules[i]);
			if (!lsys_copy->rules[i]) {
				fprintf(stderr, "copy_strdup() failed\n");
				for (int j = 0; j < i; j++) {
					free(lsys_copy->rules[j]);
				}
				free(lsys_copy->axiom);
				free(lsys_copy);
				return NULL;
			}
		}
	}

	return lsys_copy;
}

state_t state_deep_copy(state_t *state)
{
	state_t state_copy;
	state_copy.img = NULL;
	state_copy.font = NULL;
	state_copy.lsys  = NULL;

	if (state->img) {
		state_copy.img = image_deep_copy(state->img);
		if (!state_copy.img) {
			return state_copy;
		}
	}

	if (state->font) {
		state_copy.font = font_deep_copy(state->font);
		if (!state_copy.font) {
			free_image(state_copy.img);
			state_copy.img = NULL;
			return state_copy;
		}
	}

	if (state->lsys) {
		state_copy.lsys = lsystem_deep_copy(state->lsys);
		if (!state_copy.lsys) {
			free_image(state_copy.img);
			state_copy.img = NULL;
			free_font(state_copy.font);
			state_copy.font = NULL;
			return state_copy;
		}
	}

	return state_copy;
}

/* adauga in stiva de memorie o noua intrare - retinand comanda (numele ei) si
 fisierul pe care il prelucreaza (daca e cazul, pentru a se afisa eventuale
  mesaje la redo) */
int stack_add(stack_t *stack, state_t *current_state, char *command,
			  char *file_name)
{
	state_record_t entry_record;

	entry_record.state = state_deep_copy(current_state);
	entry_record.command = copy_strdup(command);
	if (!entry_record.command) {
		fprintf(stderr, "copy_strdup() failed\n");
		free_state(&entry_record.state);
		return 0;
	}

	if (file_name) {
		entry_record.file_name = copy_strdup(file_name);
		if (!entry_record.file_name) {
			fprintf(stderr, "copy_strdup() failed\n");
			free(entry_record.command);
			free_state(&entry_record.state);
			return 0;
		}
	} else {
		entry_record.file_name = NULL;
	}

	state_record_t *aux = realloc(stack->state, (stack->size + 1) *
								  sizeof(state_record_t));
	if (!aux) {
		fprintf(stderr, "realloc() failed\n");
		free(entry_record.command);
		free(entry_record.file_name);
		free_state(&entry_record.state);
		return 0;
	}

	stack->state = aux;
	stack->state[stack->size++] = entry_record;
	return 1;
}

/* elimina ultima stare din stiva de memorie a programului si scade dimensiunea
 acesteia, eliberand memoria corespunzatoare */
int stack_remove(stack_t *stack, state_record_t *last_record)
{
	if (stack->size == 0)
		return 0;

	*last_record = stack->state[--stack->size];

	if (stack->size == 0) {
		free(stack->state);
		stack->state = NULL;
	} else {
		state_record_t *aux = realloc(stack->state, stack->size *
									  sizeof(state_record_t));
		if (!aux) {
			fprintf(stderr, "realloc() failed\n");
			return 0;
		}
		stack->state = aux;
	}
	return 1;
}

/* realizeaza comanda de undo, eliminand ultima stare din stiva de comenzi
introduse care sunt undoable si o adauga in stiva de comenzi ce pot fi redoable
iar starea anterioara devine starea curenta*/
int undo(stack_t *undoed_stack, stack_t *redoed_stack, state_t *current_state)
{
	state_record_t previous_record;

	int can_remove_state = stack_remove(undoed_stack, &previous_record);
	if (can_remove_state == 0) {
		return 0;
	}

	stack_add(redoed_stack, current_state, previous_record.command,
			  previous_record.file_name);
	free_state(current_state);

	*current_state = previous_record.state;

	free(previous_record.command);
	free(previous_record.file_name);
	return 1;
}

/* realizeaza comanda de redo, restaurand utima stare si eliminand-o dn stiva
de comezni redoable si adaugand-o in stiva de comenzi ce pot fi undoable, iar
starea extrasa devine starea actuala a programului si se afiseaza mesajul
corespunzator comenzii reincarcate daca este cazul */
int redo(stack_t *undoed_stack, stack_t *redoed_stack, state_t *current_state)
{
	state_record_t next_record;

	int can_remove_state = stack_remove(redoed_stack, &next_record);
	if (can_remove_state == 0) {
		return 0;
	}

	stack_add(undoed_stack, current_state, next_record.command,
			  next_record.file_name);
	free_state(current_state);

	*current_state = next_record.state;

	if (strcmp(next_record.command, "LSYSTEM") == 0) {
		printf("Loaded %s (L-system with %d rules)\n", next_record.file_name,
			   current_state->lsys->nrules);
	} else if (strcmp(next_record.command, "LOAD") == 0) {
		printf("Loaded %s (PPM image %dx%d)\n", next_record.file_name,
			   current_state->img->w, current_state->img->h);
	} else if (strcmp(next_record.command, "TURTLE") == 0) {
		printf("Drawing done\n");
	} else if (strcmp(next_record.command, "TYPE") == 0) {
		printf("Text written\n");
	} else if (strcmp(next_record.command, "FONT") == 0) {
		printf("Loaded %s (bitmap font %s)\n", next_record.file_name,
			   current_state->font->name);
	}

	free(next_record.command);
	free(next_record.file_name);
	return 1;
}
