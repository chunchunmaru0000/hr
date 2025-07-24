#include "gner.h"

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

const char SA_JMP[] = "\tидти ";
const uint32_t SA_JMP_LEN = loa(SA_JMP);

void gen_local_Асм_Linux_64(struct Gner *g, struct Inst *in) {
	struct Token *tok, *name, *str;
	uint32_t i = 0;

	switch (in->code) {
	case IP_ASM:
		// ### os explanation:
		//   _ - assembly string token

		str = plist_get(in->os, 0);
		blat_blist(g->text, str->str);
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

		text_add('\t');
		blat_blist(g->text, g->current_function->signature);
		blat_blist(g->text, name->view);
		text_add(':');
		text_add('\n');
		break;
	case IP_GOTO:
		// ### os explanation
		//   _ - label name token

		name = plist_get(in->os, 0);

		blat_str_text(SA_JMP);
		blat_blist(g->text, g->current_function->signature);
		blat_blist(g->text, name->view);
		text_add('\n');
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
	// long stack_was = g->stack_counter;

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
			blat_str_text(SA_EQU);				  // вот
			blat_blist(g->text, var->name->view); // name
			text_add(' ');
			num_add(g->text, g->stack_counter);

			blat_str_text(SA_START_COMMENT);
			num_hex_add(g->text, g->stack_counter);

			text_add('\n');
		}

		last_offset = arg->offset;
	}

	// if (stack_was != g->stack_counter) {
	// 	blat_str_text(STR_ASM_SUB_RSP);
	// 	num_add(g->text, stack_was - g->stack_counter);
	// 	text_add('\n');
	// }
}
