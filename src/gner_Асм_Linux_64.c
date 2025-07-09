#include "gner.h"
#include <stdio.h>

char STR_ASM_SEGMENT[] = "участок чит исп\n";
char STR_ASM_LABEL_END[] = ":\n";

uint32_t STR_ASM_SEGMENT_LEN = loa(STR_ASM_SEGMENT);
uint32_t STR_ASM_LABEL_END_LEN = loa(STR_ASM_LABEL_END);

void gen_Асм_Linux_64_prolog(struct Gner *g) {
	blat_str_prol(g, STR_ASM_SEGMENT);
}

char STR_ASM_EQU[] = "вот ";
char STR_ASM_ENTER_STACK_FRAME[] = "\tтолк рбп\n\tбыть рбп рсп\n";
char STR_ASM_LEAVE_STACK_FRAME[] = "\tвыт рбп\n\tвозд\n";

uint32_t STR_ASM_EQU_LEN = loa(STR_ASM_EQU);
uint32_t STR_ASM_ENTER_STACK_FRAME_LEN = loa(STR_ASM_ENTER_STACK_FRAME);
uint32_t STR_ASM_LEAVE_STACK_FRAME_LEN = loa(STR_ASM_LEAVE_STACK_FRAME);

void gen_Асм_Linux_64_text(struct Gner *g) {
	uint32_t i, j;
	struct Inst *in;
	struct Token *tok, *tok2;
	struct BList *some_blist;
	struct GlobVar *global_var;
	struct Defn *defn;

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
			// 			tok = plist_get(in->os, 0);
			//
			// 			for (j = 1; j < in->os->size; j++) {
			// 				tok2 = plist_get(in->os, j);
			//
			// 				blat_str_bprol(g, STR_ASM_EQU);	  // вот
			// 				blat_blist(g->bprol, tok->view);  // ЧЕТО
			// 				bprol_add(g, '.');				  // .
			// 				blat_blist(g->bprol, tok2->view); // ИМЯ
			// 				bprol_add(g, ' ');				  //
			//
			// 				if (tok2->fpn == HAVE_NUM)
			// 					blat_blist(g->bprol, tok2->str);
			// 				else {
			// 					some_blist = num_to_str(j - 1);
			// 					blat_blist(g->bprol, some_blist);
			// 					blist_clear_free(some_blist);
			// 				}
			// 				bprol_add(g, '\n');
			// 			}
			for (j = 1; j < in->os->size; j++) {
				defn = plist_get(in->os, j);

				blat_str_bprol(g, STR_ASM_EQU); // вот
				blat_blist(g->bprol, defn->view);
				bprol_add(g, ' ');
				// num
				some_blist = num_to_str((long)defn->value);
				blat_blist(g->bprol, some_blist);
				blist_clear_free(some_blist);

				bprol_add(g, '\n');
			}

			bprol_add(g, '\n');
			break;
		case IP_DECLARE_FUNCTION:
			global_var = plist_get(in->os, 0);

			blat_blist(g->text, global_var->signature); // fun label
			blat_str_text(g, STR_ASM_LABEL_END);		// :
			blat_str_text(g, STR_ASM_ENTER_STACK_FRAME);

			blat_str_text(g, STR_ASM_LEAVE_STACK_FRAME);
			break;
		default:
			eei(in->f, in, "эээ", 0);
		}
	}
end_gen_Асм_text_loop:;
}
