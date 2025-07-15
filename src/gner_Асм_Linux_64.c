#include "gner.h"
#include <stdio.h>

void put_args_on_the_stack_Асм_Linux_64(struct Gner *g, struct Inst *in);

char STR_ASM_SEGMENT[] = "участок чит исп\n";
char STR_ASM_LABEL_END[] = ":\n";

uint32_t STR_ASM_SEGMENT_LEN = loa(STR_ASM_SEGMENT);
uint32_t STR_ASM_LABEL_END_LEN = loa(STR_ASM_LABEL_END);

void gen_Асм_Linux_64_prolog(struct Gner *g) { blat_str_prol(STR_ASM_SEGMENT); }

char STR_ASM_EQU[] = "вот ";
char STR_ASM_ENTER_STACK_FRAME[] = "\tтолк рбп\n\tбыть рбп рсп\n";
char STR_ASM_LEAVE_STACK_FRAME[] = "\tвыт рбп\n\tвозд\n";
char STR_ASM_MOV_MEM_RSP_OPEN[] = "\tбыть (рсп ";

uint32_t STR_ASM_EQU_LEN = loa(STR_ASM_EQU);
uint32_t STR_ASM_ENTER_STACK_FRAME_LEN = loa(STR_ASM_ENTER_STACK_FRAME);
uint32_t STR_ASM_LEAVE_STACK_FRAME_LEN = loa(STR_ASM_LEAVE_STACK_FRAME);
uint32_t STR_ASM_MOV_MEM_RSP_OPEN_LEN = loa(STR_ASM_LEAVE_STACK_FRAME);

const struct Register regs[] = {
	{"р8", 3, R_R8, QWORD},	  {"р9", 3, R_R9, QWORD},
	{"р10", 3, R_R10, QWORD}, {"р11", 3, R_R11, QWORD},
	{"р12", 3, R_R12, QWORD}, {"р13", 3, R_R13, QWORD},
	{"р14", 3, R_R14, QWORD}, {"р15", 3, R_R15, QWORD},
};

void gen_Асм_Linux_64_text(struct Gner *g) {
	uint32_t i, j;
	struct Inst *in;
	struct Token *tok, *tok2;
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
			for (j = 1; j < in->os->size; j++) {
				defn = plist_get(in->os, j);

				blat_str_bprol(STR_ASM_EQU); // вот
				blat_blist(g->bprol, defn->view);
				bprol_add(' ');
				num_add(g->bprol, (long)defn->value);
				bprol_add('\n');
			}

			bprol_add('\n');
			break;
		case IP_DECLARE_FUNCTION:
			g->stack_counter = 0;
			// TODO: maybe free em cuz they are in no need anywhere after
			plist_clear(g->local_vars);

			global_var = plist_get(in->os, 0);

			blat_blist(g->text, global_var->signature); // fun label
			blat_str_text(STR_ASM_LABEL_END);			// :
			blat_str_text(STR_ASM_ENTER_STACK_FRAME);

			// put args on the stack
			// put_args_on_the_stack_Асм_Linux_64(g, in);
			// function body

			// g->stack_counter = 0;
			// free stack in return statement
			blat_str_text(STR_ASM_LEAVE_STACK_FRAME);
			break;
		default:
			eei(in->f, in, "эээ", 0);
		}
	}
end_gen_Асм_text_loop:;
}

void put_args_on_the_stack_Асм_Linux_64(struct Gner *g, struct Inst *in) {
	// fun in->os are: fun va riable, args..., 0 term, ...
	uint32_t i, j;
	struct FunArg *arg = plist_get(in->os, 1);
	struct LocalVar *var;
	int argSize;

	for (i = 2; arg; i++) {
		// size of arg and eithers are equal by done so in pser
		argSize = get_type_code_size(arg->type->code);
		g->stack_counter -= argSize;

		for (j = 0; j < arg->names->size; j++) {
			var = new_local_var(plist_get(arg->names, j), arg->type,
								g->stack_counter);
			plist_add(g->local_vars, var);

			g->tmp_blist = num_to_str(g->stack_counter);

			blat_str_text(STR_ASM_EQU);			  // вот
			blat_blist(g->text, var->name->view); // name
			text_add(' ');
			blat_blist(g->text, g->tmp_blist); // stack ptr
			text_add('\n');

			// mov [rsp - g->tmp_blist], register
		}

		arg = plist_get(in->os, i);
	}

	if (g->stack_counter) {
		// sub rps g->stack_counter
		// num_add(g, g->text, g->stack_counter);
	}
}
