#include "gner.h"

struct Gner *new_gner(struct Pser *p, enum Target tget, uc debug) {
	struct Gner *g = malloc(sizeof(struct Gner));

	g->t = tget;
	g->debug = debug;

	g->is = pse(p);
	g->pos = 0;
	g->stack_counter = 0;
	g->flags = malloc(sizeof(struct Fggs));

	g->bprol = new_blist(128);
	g->prol = new_blist(128);
	g->text = new_blist(128);
	g->global_vars = p->global_vars;
	g->local_vars = new_plist(16);
	g->local_labels = new_plist(8);

	g->tmp_blist = 0;

	free(p);
	return g;
}

void gen(struct Gner *g) {
	switch (g->t) {
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

void free_and_clear_local_vars(struct Gner *g) {
	struct LocalVar *var;
	uint32_t i, j = 0; //, last_j_start = 0;
	struct PList *freed_type_pointers = new_plist(g->local_vars->size);

	for (i = 0; i < g->local_vars->size; i++) {
		var = plist_get(g->local_vars, i);

		for (/*j = last_j_start*/; j < freed_type_pointers->size; j++) {
			if (var->type == plist_get(freed_type_pointers, j)) {
				// last_j_start optimizes search loop cuz vars with same type
				// can occur only after already freed one and not before
				// last_j_start = j;
				// last_j_start like is kunda equal to last j loop so
				// its useless
				goto skip_freed_pointer_free;
			}
		}

		free_type(var->type);
		plist_add(freed_type_pointers, var->type);

	skip_freed_pointer_free:;
		free(var);
	}

	plist_free(freed_type_pointers);
	plist_clear(g->local_vars);
}
