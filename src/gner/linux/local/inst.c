#include "../../gner.h"
#include <stdio.h>

void put_vars_on_the_stack(struct Gner *g, struct Inst *in);

constr CHANGE_VAR_NAME_OR_DELETE_VAR = "изменить имя переменной или удалить ее";
constr CHANGE_LABEL_NAME_OR_DELETE_LABEL = "изменить имя метки или удалить ее";
constr REDEFINING_OF_LOCAL_VAR = "Переопределение локальной переменной.";
constr REDEFINING_OF_LOCAL_LABEL = "Переопределение локальной метки.";

void gen_block(Gg, struct PList *os) {
	g->indent_level++;
	for (u32 i = 0; i < os->size; i++)
		gen_local_linux(g, plist_get(os, i));
	g->indent_level--;
}

void gen_local_linux(struct Gner *g, struct Inst *in) {
	struct Token *tok, *name, *str;
	struct BList *string;
	struct Loop *loop;
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
			blat_ft(name->view), ft_add(':'), ft_add('\n');
		}
		g->indent_level++;
		break;
	case IP_GOTO:
		// ### os explanation
		//   _ - label name token

		name = plist_get(in->os, 0);

		op_(JMP);
		blat_ft(g->current_function->signature);
		blat_ft(name->view);
		ft_add('\n');
		break;
	case IP_LOOP:
		// ### os explanation
		// ... - local instructions
		//   _ - struct Loop *

		loop = p_last(in->os);
		in->os->size--; // remove loop from os

		string = loop->cont ? loop->cont : take_label(LC_LOOP);
		add_label(string);

		gen_block(g, in->os);
		jmp_(string);

		blist_clear_free(string); // free cont
		if (loop->brek) {
			add_label(loop->brek);
			blist_clear_free(string); // free brek
		}
		free(loop);
		break;
	case IP_LOCAL_EXPRESSION:
		gen_local_expr_inst_linux(g, in);
		break;
	case IP_BREAK:
	case IP_CONTINUE:
		// ### os explanation
		//   _ - label to jmp to

		string = plist_get(in->os, 0);
		jmp_(string);
		blist_clear_free(string);
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
