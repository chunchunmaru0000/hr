#include "gner.h"

void put_vars_on_the_stack_Асм_Linux_64(struct Gner *g, struct Inst *in);

const char REDEFINING_OF_LOCAL_VAR[] = "Переопределение локальной переменной.";
const char CHANGE_VAR_NAME_OR_DELETE_VAR[] =
	"изменить имя переменной или удалить ее";

const uint32_t REDEFINING_OF_LOCAL_VAR_LEN = loa(REDEFINING_OF_LOCAL_VAR);
const uint32_t CHANGE_VAR_NAME_OR_DELETE_VAR_LEN =
	loa(CHANGE_VAR_NAME_OR_DELETE_VAR);

void gen_local_Асм_Linux_64(struct Gner *g, struct Inst *in) {
	struct Token *tok;

	switch (in->code) {
	case IP_ASM:
		// ### os explanation:
		//   _ - assembly string token

		tok = plist_get(in->os, 0);
		blat_blist(g->text, tok->str);
		break;
	case IP_LET:
		// ### os explanation
		// ... - Arg's

		put_vars_on_the_stack_Асм_Linux_64(g, in);
		break;
	case IP_NONE:
	default:
		eei(in->f, in, "эээ", 0);
	}
}

void put_vars_on_the_stack_Асм_Linux_64(struct Gner *g, struct Inst *in) {
	uint32_t i, j, vars;
	struct Arg *arg;
	struct LocalVar *var;
	long last_offset = -1;
	long stack_was = g->stack_counter;

	for (i = 0; i < in->os->size; i++) {
		arg = plist_get(in->os, i);
		if (arg->offset != last_offset)
			g->stack_counter -= size_of_type(arg->type);

		for (j = 0; j < arg->names->size; j++) {
			var = new_local_var(plist_get(arg->names, j), arg->type,
								g->stack_counter);

			for (vars = 0; vars < g->local_vars->size; vars++) {
				if (sc((char *)((struct LocalVar *)plist_get(g->local_vars,
															 vars))
						   ->name->view->st,
					   (char *)var->name->view->st))
					eet(in->f, var->name, REDEFINING_OF_LOCAL_VAR,
						CHANGE_VAR_NAME_OR_DELETE_VAR);
			}

			plist_add(g->local_vars, var);

			text_add('\t');
			blat_str_text(STR_ASM_EQU);			  // вот
			blat_blist(g->text, var->name->view); // name
			text_add(' ');
			num_add(g->text, g->stack_counter);

			blat_str_text(STR_ASM_START_COMMENT);
			num_hex_add(g->text, g->stack_counter);

			text_add('\n');
		}

		last_offset = arg->offset;
	}

	if (stack_was != g->stack_counter) {
		blat_str_text(STR_ASM_SUB_RSP);
		num_add(g->text, stack_was - g->stack_counter);
		text_add('\n');
	}
}
