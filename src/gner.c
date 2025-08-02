#include "gner.h"

struct Gner *new_gner(struct Pser *p, enum Target tget, uc debug) {
	struct Gner *g = malloc(sizeof(struct Gner));

	g->t = tget;
	g->debug = debug;

	g->is = pse(p);
	g->indent_level = 0;
	g->pos = 0;
	g->stack_counter = 0;

	g->enums = p->enums;
	g->structs = p->structs;
	g->flags = malloc(sizeof(struct Fggs));

	g->labels = malloc(sizeof(struct Lbls));
	g->labels->loops = 0;
	g->labels->whiles = 0;
	g->labels->fors = 0;
	g->labels->ifs = 0;
	g->labels->elses = 0;

	g->bprol = new_blist(128);
	g->prol = new_blist(128);
	g->text = new_blist(128);
	g->fun_prol = new_blist(128);
	g->fun_text = new_blist(128);

	g->global_vars = p->global_vars;
	g->local_vars = new_plist(16);
	g->local_labels = new_plist(8);

	g->tmp_blist = 0;

	// TODO: free pser properly
	free(p);
	return g;
}

void gen(struct Gner *g) {
	switch (g->t) {
	case T_Асм_Linux_64:
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

void free_and_clear_local_vars(struct Gner *g) {
	struct LocalVar *var;
	uint32_t i;
	void *last_freed = 0;

	for (i = 0; i < g->local_vars->size; i++) {
		var = plist_get(g->local_vars, i);

		if (var->type == last_freed)
			goto skip_freed_pointer_free;

		free_type(var->type);
		last_freed = var->type;

	skip_freed_pointer_free:;
		free(var);
	}

	plist_clear(g->local_vars);
}

void write_fun(struct Gner *g) {
	blat_blist(g->text, g->fun_prol);
	blat_blist(g->text, g->fun_text);

	blist_clear(g->fun_prol);
	blist_clear(g->fun_text);
}

void indent_line(struct Gner *g, struct BList *l) {
	for (uint32_t i = g->indent_level; i; i--)
		blist_add(l, '\t');
}
