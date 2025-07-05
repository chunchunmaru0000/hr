#include "gner.h"
#include <stdio.h>

char STR_ASM_SEGMENT[] = "участок чит исп\n";

void gen_Асм_Linux_64_prolog(struct Gner *g) {
	blat_str_prol(g, STR_ASM_SEGMENT, loa(STR_ASM_SEGMENT));
}

char STR_ASM_EQU[] = "вот ";

void gen_Асм_Linux_64_text(struct Gner *g) {
	uint32_t i, j;
	struct Inst *in;
	struct Token *tok, *tok2;
	struct BList *some_blist;

	for (i = 0; i < g->is->size; i++) {
		in = plist_get(g->is, i);
		g->pos = i;

		switch (in->code) {
		case IP_EOI:
			goto end_gen_Асм_text_loop;
		case IP_ASM:
			tok = plist_get(in->os, 0);
			blat_blist(g->text, tok->str);
			break;
		case IP_DECLARE_ENUM:
			tok = plist_get(in->os, 0);

			for (j = 1; j < in->os->size; j++) {
				tok2 = plist_get(in->os, j);

				blat_str_bprol(g, STR_ASM_EQU, loa(STR_ASM_EQU)); // вот
				blat_blist(g->bprol, tok->view);				  // ЧЕТО
				bprol_add(g, '.');								  // .
				blat_blist(g->bprol, tok2->view);				  // ИМЯ
				bprol_add(g, ' ');								  //

				if (tok2->fpn == HAVE_NUM)
					blat_blist(g->bprol, tok2->str);
				else {
					some_blist = num_to_str(j - 1);
					blat_blist(g->bprol, some_blist);
					blist_clear_free(some_blist);
				}
				bprol_add(g, '\n');
			}

			bprol_add(g, '\n');
			break;
		default:
			ee(in->f, in->p, "эээ");
		}
	}
end_gen_Асм_text_loop:;
}
