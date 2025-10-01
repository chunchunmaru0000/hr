#include "tzer.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void print_spaces(int n) {
	while (n--)
		putchar(' ');
}

const char *const COLOR_BLACK = "\x1B[30m";
const char *const COLOR_LIGHT_BLACK = "\033[90m";
const char *const COLOR_LIGHT_RED = "\x1B[91m";
const char *const COLOR_RED = "\x1B[31m";
const char *const COLOR_GREEN = "\x1B[32m";
const char *const COLOR_YELLOW = "\x1B[33m";
const char *const COLOR_BLUE = "\x1B[34m";
const char *const COLOR_PURPLE = "\x1B[35m";
const char *const COLOR_LIGHT_PURPLE = "\033[95m";
const char *const COLOR_GAY = "\x1B[36m";
const char *const COLOR_WHITE = "\x1B[37m";
const char *const COLOR_RESET = "\x1B[0m";
const int TEXT_TAB_SPACES = 4;
const char UNDERLINE_CHAR = '~';

const char *write_ln(const char *line) {
	while (*line && *line != '\n') {
		if (*line == '\t')
			print_spaces(TEXT_TAB_SPACES);
		else
			putchar(*line);
		line++;
	}
	if (*line == '\n')
		line++;
	return line;
}

uint32_t get_utf8_chars_to_pos(const char *str, int col) {
	uint32_t chars = 0;
	// do it good with proper 0b10... and all
	// its works so why would i
	for (; col > 0; col--) {
		if (*str == '\t')
			chars += TEXT_TAB_SPACES - 1;
		else if (*str & 0b10000000) {
			if ((*str & 0b11100000) == 0b11000000) {
				str += 1;
				col -= 1;
			} else if ((*str & 0b11110000) == 0b11100000) {
				str += 2;
				col -= 2;
			} else if ((*str & 0b11111000) == 0b11110000) {
				str += 3;
				col -= 3;
			}
		}
		chars++;
		str++;
	}
	return chars;
}

const char *get_line_start(struct Pos *p) {
	const char *str_start = p->f->code;
	uint32_t line = p->line - 1;
	uint32_t nc = line;

	if (line)
		nc--;
	for (; nc; str_start++) {
		if (*str_start == '\n')
			nc--;
	}
	return str_start;
}

void print_source_line(struct Pos *p, const char *const color, char *help) {
	char *help_end, *help_start = help;
	uint32_t ut8_chars_to_pos = 0, line = p->line - 1;
	const char *str_start = get_line_start(p);

	printf("%s%5d |", COLOR_RESET, line);
	if (line)
		str_start = write_ln(str_start);
	putchar('\n');

	ut8_chars_to_pos = get_utf8_chars_to_pos(str_start, p->col - 1);

	printf("%5d |%s", line + 1, color);
	str_start = write_ln(str_start);
	printf("%s\n", COLOR_RESET);

	if (help) {
		help_end = help;
		// this loop prints every line in help with offset
		// of spaces of ut8_chars_to_pos
		while (*help_end) {
			while (*help_end && *help_end != '\n')
				help_end++;
			// printf("\t\t|%d|%s|\n", help_end[0], help);
			if (*help_end) {   // if not str end
				*help_end = 0; // terminate line

				printf("      |%s", COLOR_GREEN);
				print_spaces(ut8_chars_to_pos);
				printf("%s%s\n", help, COLOR_RESET);

				help_end++;
				help = help_end;
			} else {
				printf("      |%s", COLOR_GREEN);
				print_spaces(ut8_chars_to_pos);
				printf("%s%s\n", help, COLOR_RESET);
			}
		}

		free(help_start); // its malloced so in case of warns
	}

	printf("%5d |", line + 2);
	if (*str_start)
		write_ln(str_start);
	putchar('\n');
}

struct ErrorInfo *new_error_info(struct Token *t, const char *const msg,
								 const char *const sgst) {
	struct ErrorInfo *ei = malloc(sizeof(struct ErrorInfo));
	ei->t = t;
	ei->msg = msg;
	ei->sgst = sgst;
	ei->extra = 0;
	ei->extra_type = ET_NONE;
	return ei;
}

void ee(struct Pos *p, const char *const msg) { // error exit
	fprintf(stderr, "%s%s:%d:%d %sОШИБКА: %s%s\n", COLOR_WHITE, p->f->path,
			p->line, p->col, COLOR_RED, msg, COLOR_RESET);
	print_source_line(p, COLOR_LIGHT_RED, 0);
	exit(1);
}

void et(struct Token *t, const char *const msg, const char *const sgst) {
	char *help;
	uint32_t token_chars_len, help_len, sgst_len = -1;

	sgst_len = sgst ? strlen(sgst) : t->view->size; // strlen(vs(t))
	// if (sgst)
	// 	sgst_len = strlen(sgst);

	token_chars_len =
		get_utf8_chars_to_pos((const char *)t->view->st, t->view->size);

	help_len = token_chars_len + 1 + sgst_len;
	help = malloc(help_len + 1);

	help[help_len] = 0; // terminate
	for (uint32_t i = 0; i < token_chars_len; i++)
		help[i] = UNDERLINE_CHAR; // fill with underline

	help[token_chars_len] = '\n'; // split by \n
	memcpy(help + token_chars_len + 1, sgst ? sgst : vs(t), sgst_len);
	// if (sgst) {
	// 	help[token_chars_len] = '\n'; // split by \n
	// 	memcpy(help + token_chars_len + 1, sgst, sgst_len);
	// }

	fprintf(stderr, "%s%s:%d:%d %sОШИБКА: %s\n", COLOR_WHITE, t->p->f->path,
			t->p->line, t->p->col, COLOR_RED, msg);
	print_source_line(t->p, COLOR_LIGHT_RED, help);
}

void eet(struct Token *t, const char *const msg, const char *const sgst) {
	et(t, msg, sgst);
	exit(1);
}

void eet2(struct Token *t0, struct Token *t1, const char *const msg,
		  const char *const sgst) {
	et(t0, msg, sgst);

	fprintf(stderr, "%s%s:%d:%d %sВ: %s\n", COLOR_WHITE, t1->p->f->path,
			t1->p->line, t1->p->col, COLOR_GREEN, msg);
	print_source_line(t1->p, COLOR_GREEN, vs(t1));

	exit(1);
}
