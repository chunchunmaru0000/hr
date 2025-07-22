#include "gner.h"

void gen_local_Асм_Linux_64(struct Gner *g, struct Inst *in) {
	struct Token *tok;

	switch (in->code) {
	case IP_ASM:
		// ### os explanation:
		//   _ - assembly string token

		tok = plist_get(in->os, 0);
		blat_blist(g->text, tok->str);
		break;
	case IP_NONE:
	default:
		eei(in->f, in, "эээ", 0);
	}
}
