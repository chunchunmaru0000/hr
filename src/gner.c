#include "gner.h"

struct Gner *new_gner(struct Pser *p, enum Target tget, uc debug) {
	struct Gner *g = malloc(sizeof(struct Gner));

	g->t = tget;
	g->debug = debug;

	g->is = pse(p);
	g->pos = 0;
	g->stack_counter = 0;

	g->bprol = new_blist(128);
	g->prol = new_blist(128);
	g->text = new_blist(128);
	g->global_vars = p->global_vars;
	g->local_vars = new_plist(16);

	g->tmp_blist = 0;

	free(p);
	return g;
}

void gen(struct Gner *g) {
	switch (g->t) {
	case T_Fasm_Linux_64:
		gen_Fasm_Linux_64_prolog(g);
		gen_Fasm_Linux_64_text(g);
		break;
	case T_Асм_Linux_64:
		gen_Асм_Linux_64_prolog(g);
		gen_Асм_Linux_64_text(g);
		break;
	}
}

struct LocalVar *new_local_var(struct Token *name, struct TypeExpr *type,
							   long stack_pointer) {
	struct LocalVar *var = malloc(sizeof(struct LocalVar));
	var->name = name;
	var->type = type;
	var->stack_pointer = stack_pointer;
	return var;
}
