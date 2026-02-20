
#ifndef UNDO_REDO_H
#define UNDO_REDO_H

#include "ppm_image.h"
#include "lsystem.h"
#include "font.h"
#include "turtle.h"

typedef struct {
	image_t *img;
	font_t *font;
	lsystem_t *lsys;
} state_t;

typedef struct {
	state_t state;
	char *command;
	char *file_name;
} state_record_t;

typedef struct {
	state_record_t *state;
	int size;
} stack_t;

void free_glyph(character_t *glyph);

void free_image(image_t *img);

void free_font(font_t *font);

void free_lsystem(lsystem_t *lsys);

void free_state(state_t *state);

void free_stack(stack_t *stack);

image_t *image_deep_copy(image_t *img);

int character_deep_copy(character_t *character_copy, character_t *character);

font_t *font_deep_copy(font_t *font);

lsystem_t *lsystem_deep_copy(lsystem_t *lsys);

state_t state_deep_copy(state_t *state);

int stack_add(stack_t *stack, state_t *current_state, char *command,
			  char *file_name);

int stack_remove(stack_t *stack, state_record_t *last_record);

int undo(stack_t *undoed_stack, stack_t *redoed_stack, state_t *current_state);

int redo(stack_t *undoed_stack, stack_t *redoed_stack, state_t *current_state);

#endif
