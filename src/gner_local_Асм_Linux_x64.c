#include "gner.h"
#include <stdio.h>

void put_vars_on_the_stack_Асм_Linux_64(struct Gner *g, struct Inst *in);

const char CHANGE_VAR_NAME_OR_DELETE_VAR[] =
	"изменить имя переменной или удалить ее";
const char CHANGE_LABEL_NAME_OR_DELETE_LABEL[] =
	"изменить имя метки или удалить ее";
const char REDEFINING_OF_LOCAL_VAR[] = "Переопределение локальной переменной.";
const char REDEFINING_OF_LOCAL_LABEL[] = "Переопределение локальной метки.";

const uint32_t CHANGE_VAR_NAME_OR_DELETE_VAR_LEN =
	loa(CHANGE_VAR_NAME_OR_DELETE_VAR);
const uint32_t CHANGE_VAR_NAME_OR_DELETE_LABEL_LEN =
	loa(CHANGE_LABEL_NAME_OR_DELETE_LABEL);

const uint32_t REDEFINING_OF_LOCAL_VAR_LEN = loa(REDEFINING_OF_LOCAL_VAR);
const uint32_t REDEFINING_OF_LOCAL_LABEL_LEN = loa(REDEFINING_OF_LOCAL_LABEL);

const char SA_JMP[] = "идти ";
const uint32_t SA_JMP_LEN = loa(SA_JMP);

void gen_local_Асм_Linux_64(struct Gner *g, struct Inst *in) {
	struct Token *tok, *name, *str;
	struct BList *string;
	uint32_t i = 0;

	switch (in->code) {
	case IP_ASM:
		// ### os explanation:
		//   _ - assembly string token

		str = plist_get(in->os, 0);
		blat_blist(g->fun_text, str->str);
		break;
	case IP_LET:
		// ### os explanation
		// ... - Arg's

		put_vars_on_the_stack_Асм_Linux_64(g, in);
		break;
	case IP_DECLARE_LABEL:
		// ### os explanation
		//   _ - label name token

		name = plist_get(in->os, 0);

		for (; i < g->local_labels->size; i++) {
			tok = plist_get(g->local_labels, i);
			if (sc((char *)tok->view->st, (char *)name->view->st))
				eet(in->f, name, REDEFINING_OF_LOCAL_LABEL,
					CHANGE_LABEL_NAME_OR_DELETE_LABEL);
		}
		plist_add(g->local_labels, name);

		indent_line(g, g->fun_text);
		blat_blist(g->fun_text, g->current_function->signature);
		blat_blist(g->fun_text, name->view);
		fun_text_add(':');
		fun_text_add('\n');
		break;
	case IP_GOTO:
		// ### os explanation
		//   _ - label name token

		name = plist_get(in->os, 0);

		iprint_fun_text(SA_JMP);
		blat_blist(g->fun_text, g->current_function->signature);
		blat_blist(g->fun_text, name->view);
		fun_text_add('\n');
		break;
	case IP_LOOP:
		// ### os explanation
		// ... - local instructions

		string = take_label(g, LC_LOOP);
		indent_line(g, g->fun_text);
		blat_blist(g->fun_text, string);
		fun_text_add(':');
		fun_text_add('\n');

		g->indent_level++;

		for (i = 0; i < in->os->size; i++)
			gen_local_Асм_Linux_64(g, plist_get(in->os, i));

		iprint_fun_text(SA_JMP);
		blat_blist(g->fun_text, string);
		fun_text_add('\n');

		g->indent_level--;
		blist_clear_free(string);
		break;
	case IP_NONE:
	default:
		eei(in, "эээ", 0);
	}
}

void put_vars_on_the_stack_Асм_Linux_64(struct Gner *g, struct Inst *in) {
	uint32_t i, j, vars;
	struct Arg *arg;
	struct LocalVar *var;
	long last_offset = -1;

	for (i = 0; i < in->os->size; i++) {
		arg = plist_get(in->os, i);
		if (arg->offset != last_offset)
			g->stack_counter -= arg->arg_size;

		for (j = 0; j < arg->names->size; j++) {
			var =
				new_local_var(plist_get(arg->names, j), arg, g->stack_counter);

			for (vars = 0; vars < g->local_vars->size; vars++) {
				if (sc((char *)((struct LocalVar *)plist_get(g->local_vars,
															 vars))
						   ->name->view->st,
					   (char *)var->name->view->st))
					eet(in->f, var->name, REDEFINING_OF_LOCAL_VAR,
						CHANGE_VAR_NAME_OR_DELETE_VAR);
			}

			plist_add(g->local_vars, var);

			iprint_fun_text(SA_EQU);				  // вот
			blat_blist(g->fun_text, var->name->view); // name
			fun_text_add(' ');
			int_add(g->fun_text, g->stack_counter);

			blat_str_fun_text(SA_START_COMMENT);
			hex_int_add(g->fun_text, g->stack_counter);

			fun_text_add('\n');
		}

		last_offset = arg->offset;
	}

	// if (stack_was != g->stack_counter) {
	// 	blat_str_text(STR_ASM_SUB_RSP);
	// 	num_add(g->text, stack_was - g->stack_counter);
	// 	text_add('\n');
	// }
}

// # - число
// _в# - вечно
// _п# - пока
// _д# - для
// _е# - если
// _и# - иначе
#define LETTER_LEN 3
const char *const LETTER_LOOP = "_в";
const char *const LETTER_WHILE = "_п";
const char *const LETTER_FOR = "_д";
const char *const LETTER_IF = "_е";
const char *const LETTER_ELSE = "_и";
const char *const LETTER_PTR = "_у";

struct BList *take_label(struct Gner *g, enum L_Code label_code) {
	struct BList *label = new_blist(8), *num;

	switch (label_code) {
	case LC_LOOP:
		num = int_to_str(g->labels->loops);
		blat(label, (uc *)LETTER_LOOP, LETTER_LEN);
		g->labels->loops++;
		break;
	case LC_WHILE:
		num = int_to_str(g->labels->whiles);
		blat(label, (uc *)LETTER_WHILE, LETTER_LEN);
		g->labels->whiles++;
		break;
	case LC_FOR:
		num = int_to_str(g->labels->fors);
		blat(label, (uc *)LETTER_FOR, LETTER_LEN);
		g->labels->fors++;
		break;
	case LC_IF:
		num = int_to_str(g->labels->ifs);
		blat(label, (uc *)LETTER_IF, LETTER_LEN);
		g->labels->ifs++;
		break;
	case LC_ELSE:
		num = int_to_str(g->labels->elses);
		blat(label, (uc *)LETTER_ELSE, LETTER_LEN);
		g->labels->elses++;
		break;
	case LC_PTR:
		num = int_to_str(g->labels->ptrs);
		blat(label, (uc *)LETTER_PTR, LETTER_LEN);
		g->labels->ptrs++;
		break;
	default:
		printf("asdf 228\n");
		exit(228);
	}

	blat_blist(label, num);
	blist_clear_free(num);

	blist_cut(label);
	convert_blist_to_blist_from_str(label);

	return label;
}
