#include "../../gner.h"
#include <stdio.h>

void put_vars_on_the_stack(struct Gner *g, struct Inst *in);

constr CHANGE_VAR_NAME_OR_DELETE_VAR = "изменить имя переменной или удалить ее";
constr CHANGE_LABEL_NAME_OR_DELETE_LABEL = "изменить имя метки или удалить ее";
constr REDEFINING_OF_LOCAL_VAR = "Переопределение локальной переменной.";
constr REDEFINING_OF_LOCAL_LABEL = "Переопределение локальной метки.";

void gen_local_linux(struct Gner *g, struct Inst *in) {
	struct Token *tok, *name, *str;
	struct BList *string;
	uint32_t i = 0;

	switch (in->code) {
	case IP_ASM:
		// ### os explanation:
		//   _ - assembly string token

		str = plist_get(in->os, 0);
		blat_ft(str->str);
		break;
	case IP_LET:
		// ### os explanation
		// ... - Arg's

		put_vars_on_the_stack(g, in);
		break;
	case IP_DECLARE_LABEL:
		// ### os explanation
		//   _ - label name token

		name = plist_get(in->os, 0);

		for (; i < g->local_labels->size; i++) {
			tok = plist_get(g->local_labels, i);
			if (vc(tok, name))
				eet(name, REDEFINING_OF_LOCAL_LABEL,
					CHANGE_LABEL_NAME_OR_DELETE_LABEL);
		}
		plist_add(g->local_labels, name);

		g->indent_level--;
		{
			indent_line(g, g->fun_text);
			blat_ft(g->current_function->signature);
			blat_ft(name->view);
			ft_add(':'), ft_add('\n');
		}
		g->indent_level++;
		break;
	case IP_GOTO:
		// ### os explanation
		//   _ - label name token

		name = plist_get(in->os, 0);

		isprint_ft(JMP);
		blat_ft(g->current_function->signature);
		blat_ft(name->view);
		ft_add('\n');
		break;
	case IP_LOOP:
		// ### os explanation
		// ... - local instructions

		string = take_label(g, LC_LOOP);
		indent_line(g, g->fun_text);
		blat_ft(string);
		ft_add(':'), ft_add('\n');

		g->indent_level++;

		for (i = 0; i < in->os->size; i++)
			gen_local_linux(g, plist_get(in->os, i));

		isprint_ft(JMP);
		blat_ft(string);
		ft_add('\n');

		g->indent_level--;
		blist_clear_free(string);
		break;
	case IP_LOCAL_EXPRESSION:
		gen_local_expr_inst_linux(g, in);
		break;
	case IP_NONE:
	default:
		eei(in, "эээ", 0);
	}
}

void put_vars_on_the_stack(struct Gner *g, struct Inst *in) {
	uint32_t i, j, vars;
	struct Arg *arg;
	struct LocalVar *var, *tmp_var;
	long last_offset = -1;

	for (i = 0; i < in->os->size; i++) {
		arg = plist_get(in->os, i);
		if (arg->offset != last_offset)
			g->stack_counter -= arg->arg_size;

		for (j = 0; j < arg->names->size; j++) {
			var =
				new_local_var(plist_get(arg->names, j), arg, g->stack_counter);

			for (vars = 0; vars < g->local_vars->size; vars++) {
				tmp_var = plist_get(g->local_vars, vars);

				if (vc(tmp_var->name, var->name))
					eet(var->name, REDEFINING_OF_LOCAL_VAR,
						CHANGE_VAR_NAME_OR_DELETE_VAR);
			}

			plist_add(g->local_vars, var);

			iprint_after_stack_frame(SA_EQU);		 // вот
			blat_after_stack_frame(var->name->view); // name
			after_stack_frame_add(' ');
			add_int_with_hex_comm(after_stack_frame, g->stack_counter);
		}

		last_offset = arg->offset;
	}
}

// # - число
// _в# - вечно
// _п# - пока
// _д# - для
// _е# - если
// _и# - иначе
#define LETTER_LEN 3
constr LETTER_LOOP = "_в";
constr LETTER_WHILE = "_п";
constr LETTER_FOR = "_д";
constr LETTER_IF = "_е";
constr LETTER_ELSE = "_и";
constr LETTER_PTR = "_у";

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
	zero_term_blist(label);

	return label;
}
