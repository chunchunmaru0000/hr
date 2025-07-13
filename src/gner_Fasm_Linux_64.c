#include "gner.h"

char STR_FASM_FORMAT[] = "format ELF64 executable 3\n";
char STR_FASM_SEGMENT[] = "segment readable executable\n";

uint32_t STR_FASM_FORMAT_LEN = loa(STR_FASM_FORMAT);
uint32_t STR_FASM_SEGMENT_LEN = loa(STR_FASM_SEGMENT);

void gen_Fasm_Linux_64_prolog(struct Gner *g) {
	blat_str_prol(STR_FASM_FORMAT);
	blat_str_prol(STR_FASM_SEGMENT);
}

char STR_FASM_EQU[] = " equ ";

uint32_t STR_FASM_EQU_LEN = loa(STR_FASM_EQU);

void gen_Fasm_Linux_64_text(struct Gner *g) {
	uint32_t i, j;
	struct Inst *in;
	struct Token *tok, *tok2;
	struct BList *some_blist;

	for (i = 0; i < g->is->size; i++) {
		in = plist_get(g->is, i);
		g->pos = i;

		switch (in->code) {
		case IP_EOI:
			goto end_gen_Fasm_text_loop;
		case IP_ASM:
			tok = plist_get(in->os, 0);
			blat_blist(g->text, tok->str);
			break;
		case IP_DECLARE_ENUM:
			tok = plist_get(in->os, 0);

			for (j = 1; j < in->os->size; j++) {
				tok2 = plist_get(in->os, j);

				blat_blist(g->bprol, tok->view);  // ЧЕТО
				bprol_add('.');					  // .
				blat_blist(g->bprol, tok2->view); // ИМЯ
				blat_str_bprol(STR_FASM_EQU);	  // equ

				if (tok2->fpn == HAVE_NUM)
					blat_blist(g->bprol, tok2->str);
				else {
					some_blist = num_to_str(j - 1);
					blat_blist(g->bprol, some_blist);
					blist_clear_free(some_blist);
				}
				bprol_add('\n');
			}

			bprol_add('\n');
			break;
		default:
			eei(in->f, in, "eeeeerror", 0);
		}
	}
end_gen_Fasm_text_loop:;
}
