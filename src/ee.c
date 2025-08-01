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
const char UNDERLINE_CHAR = '+';

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

void print_source_line(const char *source_code, struct Pos *p,
					   const char *const color, char *help) {
	uint32_t line = p->line - 1;
	const char *str_start = source_code;
	uint32_t nc = line, ut8_chars_to_pos = 0;
	char *help_end, *help_start = help;
	if (line)
		nc--;
	while (nc) {
		if (*str_start == '\n')
			nc--;
		str_start++;
	}

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

struct ErrorInfo *new_error_info(struct Fpfc *f, struct Token *t,
								 const char *const msg,
								 const char *const sgst) {
	struct ErrorInfo *ei = malloc(sizeof(struct ErrorInfo));
	ei->f = f;
	ei->t = t;
	ei->msg = msg;
	ei->sgst = sgst;
	return ei;
}

void ee(struct Fpfc *f, struct Pos *p, const char *const msg) { // error exit
	fprintf(stderr, "%s%s:%d:%d %sОШИБКА: %s\n", COLOR_WHITE, f->path, p->line,
			p->col, COLOR_RED, msg);
	print_source_line(f->code, p, COLOR_LIGHT_RED, 0);
	exit(1);
}

void et(struct Fpfc *f, struct Token *t, const char *const msg,
		const char *const sgst) {
	char *help;
	uint32_t token_chars_len, help_len, sgst_len = -1;
	if (sgst)
		sgst_len = strlen(sgst);

	token_chars_len =
		get_utf8_chars_to_pos((const char *)t->view->st, t->view->size);

	help_len = token_chars_len + 1 + sgst_len;
	help = malloc(help_len + 1);

	help[help_len] = 0; // terminate
	for (uint32_t i = 0; i < token_chars_len; i++)
		help[i] = UNDERLINE_CHAR; // fill with underline

	if (sgst) {
		help[token_chars_len] = '\n'; // split by \n
		memcpy(help + token_chars_len + 1, sgst, sgst_len);
	}

	fprintf(stderr, "%s%s:%d:%d %sОШИБКА: %s\n", COLOR_WHITE, f->path,
			t->p->line, t->p->col, COLOR_RED, msg);
	print_source_line(f->code, t->p, COLOR_LIGHT_RED, help);
}

void eet(struct Fpfc *f, struct Token *t, const char *const msg,
		 const char *const sgst) {
	et(f, t, msg, sgst);
	exit(1);
}
