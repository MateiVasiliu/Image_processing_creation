
#include "lsystem.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* inlocuitor pentru functia strdup */
char *copy_strdup(char *string)
{
	if (!string) {
		return NULL;
	}

	int len = strlen(string);
	char *string_copy = (char *)malloc(len + 1 * sizeof(char));
	if (!string_copy) {
		fprintf(stderr, "malloc() failed\n");
		return NULL;
	}
	memcpy(string_copy, string, len + 1);

	return string_copy;
}

/* citeste liniile din fisiere fara a sti ce dimensiune are - presupun un
buffer pe care il realoc la nevoie pana se cuprinde tot textul */
char *read_text(FILE *file)
{
	char buffer[TEMP_BUFF], *line = NULL;
	int len = 0, eol = 0;

	while (eol == 0 && fgets(buffer, TEMP_BUFF, file)) {
		int size = strlen(buffer);
		if (size > 0 && buffer[size - 1] == '\n') {
			buffer[size - 1] = '\0';
			size--;
			eol = 1;
		}
		char *aux = realloc(line, len + size + 1);
		if (!aux) {
			fprintf(stderr, "realloc() failed\n");
			free(line);
			return NULL;
		}
		line = aux;
		memcpy(line + len, buffer, size);
		len += size;
		line[len] = '\0';
	}

	return line;
}

/* calculeaza lungimea sirului obtinut la urmatoarea derivare pe baza regulilor
de derivare si a dimensiunii sirului ce va fi inlocuit */
int calculate_next_len(lsystem_t *lsys, char *string)
{
	int len = 0, i = 0;
	while (string[i] != 0) {
		if (lsys->rules[(unsigned char)string[i]]) {
			len += strlen(lsys->rules[(unsigned char)string[i]]);
		} else {
			len++;
		}
		i++;
	}

	return len;
}

/* incarca in memoria programului L-system-ul extragand din fisier axioma,
numarul de reguli si regulile de derivare */
lsystem_t *lsys_load(char *file_name)
{
	FILE *lsys_file = fopen(file_name, "rt");
	if (!lsys_file) {
		fprintf(stderr, "Can't open file %s\n", file_name);
		return NULL;
	}

	lsystem_t *lsys = (lsystem_t *)calloc(1, sizeof(lsystem_t));
	if (!lsys) {
		fprintf(stderr, "calloc() failed\n");
		fclose(lsys_file);
		return NULL;
	}

	lsys->axiom = read_text(lsys_file);
	if (!lsys->axiom) {
		free(lsys);
		fclose(lsys_file);
		return NULL;
	}

	fscanf(lsys_file, "%d", &lsys->nrules);

	for (int i = 0; i < MAX_CHAR; i++) {
		lsys->rules[i] = NULL;
	}

	for (int i = 0; i < lsys->nrules; i++) {
		char symb, *succ;
		fscanf(lsys_file, " %c", &symb);
		getc(lsys_file); // elimina spatiul dintre cei doi termeni
		succ = read_text(lsys_file);
		lsys->rules[(unsigned char)symb] = copy_strdup(succ);
		if (!lsys->rules) {
			fprintf(stderr, "copy_strdup() failed\n");
			return NULL;
		}
		free(succ);
	}

	fclose(lsys_file);
	return lsys;
}

/* realizeaza derivarea sirului - pentru fiecare caracter se inlocuieste
conform regulii pana se ajunge la a n-a derivare ceruta */
char *derive(lsystem_t *lsys, int n)
{
	char *current_str = copy_strdup(lsys->axiom);
	if (!current_str) {
		fprintf(stderr, "copy_strdup() failed\n");
		return NULL;
	}
	for (int i = 0; i < n; i++) {
		int next_len = calculate_next_len(lsys, current_str);

		char *next_str = (char *)calloc(next_len + 1, sizeof(char));
		if (!next_str) {
			fprintf(stderr, "calloc() failed\n");
			free(current_str);
			return NULL;
		}

		int pos = 0, j = 0;
		while (current_str[j] != '\0') {
			char c = current_str[j];
			if (lsys->rules[(unsigned char)c]) {
				char *replace = lsys->rules[(unsigned char)c];
				int rep_len = strlen(replace);
				memcpy(next_str + pos, replace, rep_len);
				pos += rep_len;
			} else {
				next_str[pos++] = c;
			}
			j++;
		}
		next_str[pos] = '\0';

		free(current_str);
		current_str = next_str;
	}

	return current_str;
}
