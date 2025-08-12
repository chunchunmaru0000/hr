#include "tzer.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct BList EMPTY_STR_B = {"_", 0, 1, 1};
struct BList EOF_STR_B = {"_КОНЕЦ_ФАЙЛА_", 0, 23, 23}; // конец файла

struct Tzer *new_tzer(char *filename) {
	struct Tzer *t = malloc(sizeof(struct Tzer));
	struct Fpfc *f = malloc(sizeof(struct Fpfc));
	struct Pos *p = malloc(sizeof(struct Pos));
	t->f = f;
	t->p = p;

	p->line = 1;
	p->col = 1;
	t->pos = 0;

	FILE *file = fopen(filename, "r");
	if (!file)
		ee(f, p, "ОШИБКА В ОТКРЫТИИ ФАЙЛА");

	fseek(file, 0, SEEK_END);
	long size = ftell(file);
	rewind(file);

	char *text = (char *)malloc(size + 1);
	fread(text, 1, size, file);
	text[size] = '\0';
	fclose(file);

	f->path = filename;
	f->code = text;
	f->clen = size - 1;
	return t;
}

#define cur(t) ((t)->pos < (t)->f->clen ? (t)->f->code[(t)->pos] : '\0')
#define get_tzer_token(t, offset)                                              \
	((t)->pos + (offset) < (t)->f->clen ? (t)->f->code[(t)->pos + (offset)]    \
										: '\0')
char next(struct Tzer *t) {
	t->pos++;
	t->p->col++;
	return cur(t);
}

enum TCode next_line(struct Tzer *t, struct Token *token) {
	next(t);
	t->p->col = 1;
	t->p->line++;
	token->view = &EMPTY_STR_B;
	return SLASHN;
}

const char *const TOO_MUCH_DOTS = "Слишком много точек на одно не целое число";

enum TCode num_token(struct Tzer *t, struct Token *token) {
	uc c = cur(t), n;
	long start_pos = t->pos, num_len;
	enum TCode code = INT;
	char *num_view;

	int base = 10, minus_flag = 1;
	uint64_t value = 0, mnt = 0;
	double mnt_len_10_pow = 1;

	if (c == '-') {
		minus_flag = -1;
		c = next(t);
	} else if (c == '+')
		c = next(t);
	while (c == '0')
		c = next(t);
	n = get_tzer_token(t, 1);

	if (c == 'x' || c == 'X') {
		base = 16;
		c = next(t);
	} else if ((c == 0b11010001 && n == 0b10000101) ||
			   (c == 0b11010000 && n == 0b10100101)) {
		base = 16;
		next(t);
		c = next(t);
	} else if (c == 'b' || c == 'B') {
		base = 2;
		c = next(t);
	} else if (c == 0b11010000 && (n == 0b10110001 || n == 0b10010001)) {
		base = 2;
		next(t);
		c = next(t);
	}

	if (base == 10) {
		while (c != '.' && c) {
			if (c == '_') {
				c = next(t);
				continue;
			}
			if (c < '0' || c > '9') {
				token->num = value * minus_flag;
				goto __parsed;
			}
			value *= 10;
			value += c - '0';
			c = next(t);
		}
		if (c == '.')
			c = next(t);
		else {
			token->num = value * minus_flag;
			goto __parsed;
		}
		code = REAL;
		while (c) {
			if (c == '_') {
				c = next(t);
				continue;
			}
			if (c < '0' || c > '9')
				break;
			mnt *= 10;
			mnt += c - '0';
			c = next(t);
			mnt_len_10_pow *= 10.0;
		}
		token->real = (value + mnt / mnt_len_10_pow) * minus_flag;
	} else if (base == 2) {
		while (c == '_' || c == '0' || c == '1') {
			if (c != '_') {
				value <<= 1;
				value += c == '1';
			}
			c = next(t);
		}
		token->num = value * minus_flag;
	} else { // base = 16
		// абвгде
		// abcdef
		// абстиф
		loop {
			if (c == '_') {
				c = next(t);
				continue;
			}
			if (c >= '0' && c <= '9')
				value = (value << 4) + c - '0';
			else if (c >= 'A' && c <= 'F')
				value = (value << 4) + c - 'A' + 10;
			else if (c >= 'a' && c <= 'f')
				value = (value << 4) + c - 'a' + 10;
			else if (c == 0xd0) {
				n = get_tzer_token(t, 1);
				if (n == 0x90 || n == 0xb0) // А а
					value = (value << 4) + 0xa;
				else if (n == 0x91 || n == 0xb1) // Б б
					value = (value << 4) + 0xb;
				else if (n == 0x92 || n == 0xb2) // В в
					value = (value << 4) + 0xc;
				else if (n == 0x93 || n == 0xb3) // Г г
					value = (value << 4) + 0xd;
				else if (n == 0x94 || n == 0xb4) // Д д
					value = (value << 4) + 0xe;
				else if (n == 0x95 || n == 0xb5) // Е е
					value = (value << 4) + 0xf;

				else if (n == 0xa1) // С
					value = (value << 4) + 0xc;
				else if (n == 0xa2) // Т
					value = (value << 4) + 0xd;
				else if (n == 0x98 || n == 0xb8) // И и
					value = (value << 4) + 0xe;
				else if (n == 0xa4) // Ф
					value = (value << 4) + 0xf;
				else
					break;
				next(t); // skip c
			} else if (c == 0xd1) {
				n = get_tzer_token(t, 1);
				if (n == 0x81) // с
					value = (value << 4) + 0xc;
				else if (n == 0x82) // т
					value = (value << 4) + 0xd;
				else if (n == 0x84) // ф
					value = (value << 4) + 0xf;
				else
					break;
				next(t); // skip c
			} else
				break;
			c = next(t); // skip n, get next
		}
		token->num = value * minus_flag;
	}
__parsed:
	num_len = t->pos - start_pos;
	num_view = malloc(num_len + 1);
	num_view[num_len] = 0;
	strncpy(num_view, &t->f->code[start_pos], num_len);
	// printf("\t\tnum_view: %s value: %lx\n", num_view, value);

	token->view = blist_from_str(num_view, num_len);
	return code;
}

void start_line(struct Tzer *t) {
	t->p->col = 1;
	t->p->line++;
}

enum TCode str_token(struct Tzer *t, struct Token *token) {
	long start_pos = t->pos, str_len = 2;
	struct BList *str_str = new_blist(64);
	char c;

	for (c = next(t); c != '"' && c; c = next(t)) {
		if (c == '\n')
			start_line(t);

		if (c == '\\' && get_tzer_token(t, 1) == '"') {
			str_len += 2;
			next(t);

			blist_add(str_str, '"');
		} else {
			str_len++;
			blist_add(str_str, c);
		}
	}
	next(t); // skip "

	char *str_view = malloc(str_len + 1);
	str_view[str_len] = 0;
	strncpy(str_view, t->f->code + start_pos, str_len);
	token->view = blist_from_str(str_view, str_len);

	token->str = str_str;
	blist_cut(str_str);

	return STR;
}

// next_and_alloc
struct BList *naa(struct Tzer *t, char str[], long len, enum TCode *codeptr,
				  enum TCode code) {
	for (uint32_t i = 0; i < len - 1; i++)
		next(t);
	char *s = malloc(len + 1);
	memcpy(s, str, len + 1);
	*codeptr = code;

	return blist_from_str(s, len);
}
unsigned char char_in_str(char c, char *str) {
	for (int i = 0; str[i]; i++)
		if (c == str[i])
			return 1;
	return 0;
}

const char *const SHOULD_BE_UNREACHABLE = "ДОЛЖНО БЫТЬ НЕДОСТИЖИМО";

enum TCode usable_token(struct Tzer *t, struct Token *token) {
	struct BList *view;
	enum TCode code;
	enum TCode *cp = &code;
	uc c = cur(t), n = get_tzer_token(t, 1), nn = get_tzer_token(t, 2);
	next(t);

	switch (c) {
	case ':':
		view = naa(t, ":", 1, cp, COLO);
		break;
	case '!':
		view = naa(t, "!", 1, cp, EXCL);
		break;
	case '\\':
		view = naa(t, "\\", 1, cp, SLASH);
		break;
	case '+':
		if (n == '+')
			view = naa(t, "++", 2, cp, INC);
		else if (n == '=')
			view = naa(t, "+=", 2, cp, PLUSE);
		else
			view = naa(t, "+", 1, cp, PLUS);
		break;
	case '-':
		if (n == '>')
			view = naa(t, "->", 2, cp, THIN_ARROW_R);
		else if (n == '=')
			view = naa(t, "-=", 2, cp, MINUSE);
		else if (n == '-')
			view = naa(t, "--", 2, cp, DEC);
		else
			view = naa(t, "-", 1, cp, MINUS);
		break;
	case '*':
		if (n == '=')
			view = naa(t, "*=", 2, cp, MULE);
		else
			view = naa(t, "*", 1, cp, MUL);
		break;
	case '/':
		if (n == '=')
			view = naa(t, "/=", 2, cp, DIVE);
		else if (n == '/')
			view = naa(t, "//", 2, cp, SEP);
		else
			view = naa(t, "/", 1, cp, DIV);
		break;
	case '=':
		if (n == '=')
			view = naa(t, "==", 2, cp, EQUE);
		else
			view = naa(t, "=", 1, cp, EQU);
		break;
	case ',':
		view = naa(t, ",", 1, cp, COMMA);
		break;
	case '(':
		view = naa(t, "(", 1, cp, PAR_L);
		break;
	case ')':
		view = naa(t, ")", 1, cp, PAR_R);
		break;
	case '[':
		view = naa(t, "[", 1, cp, PAR_C_L);
		break;
	case ']':
		view = naa(t, "]", 1, cp, PAR_C_R);
		break;
	case '{':
		view = naa(t, "{", 1, cp, PAR_T_L);
		break;
	case '}':
		view = naa(t, "}", 1, cp, PAR_T_R);
		break;
	case '&':
		view = naa(t, "&", 1, cp, AMPER);
		break;
	case '>':
		if (n == '>') {
			if (nn == '=')
				view = naa(t, ">>=", 3, cp, SHRE);
			else
				view = naa(t, ">>", 2, cp, SHR);
		} else if (n == '=')
			view = naa(t, ">=", 2, cp, MOREE);
		else
			view = naa(t, ">", 1, cp, MORE);
		break;
	case '<':
		if (n == '<') {
			if (nn == '=')
				view = naa(t, "<<=", 3, cp, SHLE);
			else
				view = naa(t, "<<", 2, cp, SHL);
		} else if (n == '=')
			view = naa(t, "<=", 2, cp, LESSE);
		else
			view = naa(t, "<", 1, cp, LESS);
		break;
	default:
		ee(t->f, t->p, SHOULD_BE_UNREACHABLE);
	}

	token->view = view;
	return code;
}

enum TCode com_token(struct Tzer *t, struct Token *token, uc is_long) {
	long start_pos = t->pos, com_len = 1;
	next(t);
	char *com_view;

	if (is_long) {
		next(t);
		while ((cur(t) != ';' || get_tzer_token(t, 1) != ';') &&
			   cur(t) != '\0') {
			if (cur(t) == '\n')
				start_line(t);

			next(t);
			com_len++;
		}
		next(t);
		com_len++;
		next(t);
	} else
		while (cur(t) != '\n' && cur(t) != '\0') {
			next(t);
			com_len++;
		}

	com_view = malloc(com_len + 1);
	com_view[com_len] = 0;
	strncpy(com_view, t->f->code + start_pos, com_len);

	token->view = blist_from_str(com_view, com_len);
	return COM;
}

char *stop_id = " \r\t\n\"\\;:/+-*=,()[]{}<>&!";
enum TCode id_token(struct Tzer *t, struct Token *token) {
	long start_pos = t->pos, id_len = 1;
	next(t);
	char *id_view;
	while (!char_in_str(cur(t), stop_id) && cur(t) != '\0') {
		next(t);
		id_len++;
	}

	id_view = malloc(id_len + 1);
	id_view[id_len] = 0;
	strncpy(id_view, t->f->code + start_pos, id_len);

	token->view = blist_from_str(id_view, id_len);
	return ID;
}

char *usable_chars = ";:\\/+-*=,()[]{}<>&!";
char *white_space = " \r\t";
struct Token *new_token(struct Tzer *t) {
	while (char_in_str(cur(t), white_space))
		next(t);

	uc c = cur(t);
	enum TCode code;
	struct Token *token = malloc(sizeof(struct Token));
	struct Pos *p = malloc(sizeof(struct Pos));
	p->line = t->p->line;
	p->col = t->p->col;
	token->p = p;

	// every of funcs that takes token shall assign view to token
	if (c == '\0')
		code = EF;
	else if (c == ';') {
		if (get_tzer_token(t, 1) == ';')
			code = com_token(t, token, 1);
		else
			code = com_token(t, token, 0);
	} else if (c == '\n')
		code = next_line(t, token);
	else if ((c >= '0' && c <= '9') ||
			 ((c == '-' || c == '+') &&
			  (get_tzer_token(t, 1) >= '0' && get_tzer_token(t, 1) <= '9')))
		code = num_token(t, token);
	else if (c == '"')
		code = str_token(t, token);
	else if (char_in_str(c, usable_chars))
		code = usable_token(t, token);
	else
		code = id_token(t, token);

	token->code = code;
	return token;
}

struct PList *tze(struct Tzer *t, long list_cap) {
	struct PList *l = new_plist(list_cap);
	struct Token *token = new_token(t);

	while (token->code != EF) {
		if (token->code != COM && token->code != SLASHN)
			plist_add(l, token);
		token = new_token(t);
		// printf("%s:%ld:%ld:%s\n", t->filename, token->line, token->col,
		// token->view);
	}
	token->view = &EOF_STR_B;
	plist_add(l, token);

	return l;
}
