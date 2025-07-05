#include "gner.h"

struct Gner *new_gner(struct PList *is, enum Target tget, uc debug) {
	struct Gner *g = malloc(sizeof(struct Gner));

	g->t = tget;
	g->debug = debug;

	g->is = is;
	g->pos = 0;

	g->bprol = new_blist(128);
	g->prol = new_blist(128);
	g->text = new_blist(128);

	return g;
}

void gen(struct Gner *g) {
	switch (g->t) {
	case T_Fasm_Linux_64:
		gen_Fasm_Linux_64_prolog(g);
		gen_Fasm_Linux_64_text(g);
		break;
	case T_Асм_Linux_64:
		gen_Асм_Linux_64_prolog(g);
		gen_Асм_Linux_64_text(g);
		break;
	}
}

struct BList *num_to_str(long num) {
	char *num_view = malloc(11); // 0x23456789 = 10 chars + 0 term
	int four_bits;
	num_view[10] = 0;
	num_view[0] = '0';
	num_view[1] = 'x';

	for (int i = 0; i < 8; i++) {
		four_bits = ((num >> ((7 - i) * 4)) & 0b1111);
		num_view[2 + i] = four_bits + (four_bits < 0xa ? '0' : 'a' - 0xa);
	}

	return blist_from_str(num_view, 10);
}
