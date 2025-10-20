#include "../../gner.h"

uc get_var_size(struct Gner *g, struct LocalExpr *e) {
	struct LocalVar *local_var;
	struct GlobVar *global_var;
	u32 i;

	for (i = 0; i < g->local_vars->size; i++) {
		local_var = plist_get(g->local_vars, i);

		if (sc(vs(local_var->name), vs(e->tvar)))
			return local_var->lvar_size;
	}
	for (i = 0; i < g->global_vars->size; i++) {
		global_var = plist_get(g->global_vars, i);

		if (sc(vs(global_var->name), vs(e->tvar)))
			return global_var->gvar_size;
	}

	eet(e->tvar, "Не существует переменной с таким именем.", 0);
	return 0;
}

uc get_assignee_size(struct Gner *g, struct LocalExpr *e) {
	if (e->code == LE_PRIMARY_VAR)
		return get_var_size(g, e);

	eet(e->tvar, "эээээээээ", "");
	return 0;
}
