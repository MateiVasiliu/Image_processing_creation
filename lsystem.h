
#ifndef LSYSTEM_H
#define LSYSTEM_H

#include <stdio.h>

#define MAX_CHAR 256
#define TEMP_BUFF 101

typedef struct {
	char *axiom;
	int nrules;
	char *rules[MAX_CHAR];
} lsystem_t;

char *copy_strdup(char *string);

char *read_text(FILE *file);

int calculate_next_len(lsystem_t *lsys, char *string);

lsystem_t *lsys_load(char *file/*, int nr_rules*/);

char *derive(lsystem_t *lsys, int n);

#endif
