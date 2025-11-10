#include "../../gner.h"
#include <stdio.h>

struct GlobVar *find_glob_Var(struct Gner *g, struct BList *name) {
	u32 i;
	struct GlobVar *gvar;
	foreach_begin(gvar, g->global_vars);
	if (sc(vs(gvar->name), bs(name)))
		return gvar;
	foreach_end;
	return 0;
}

struct LocalVar *find_local_Var(struct Gner *g, struct BList *name) {
	u32 i;
	struct LocalVar *lvar;
	foreach_begin(lvar, g->local_vars);
	if (sc(vs(lvar->name), bs(name)))
		return lvar;
	foreach_end;
	return 0;
}

struct Inst *find_struct(struct BList *name) {
	struct Token *token_name;
	struct Inst *declare_struct_inst;
	u32 i;

	for (i = 0; i < parsed_structs->size; i++) {
		declare_struct_inst = plist_get(parsed_structs, i);
		token_name = plist_get(declare_struct_inst->os, DCLR_STRUCT_NAME);

		if (sc(vs(token_name), bs(name)))
			return declare_struct_inst;
	}
	return 0;
}

uc get_var_size(struct Gner *g, struct LocalExpr *e, struct GlobVar **gvar,
				struct LocalVar **lvar) {

	if ((*lvar = find_local_Var(g, e->tvar->view)))
		return (*lvar)->lvar_size;
	if ((*gvar = find_glob_Var(g, e->tvar->view)))
		return (*gvar)->gvar_size;

	eet(e->tvar, "Не существует переменной с таким именем.", 0);
	return 0;
}

uc get_assignee_size(struct Gner *g, struct LocalExpr *e, struct GlobVar **gvar,
					 struct LocalVar **lvar) {
	if (lcep(VAR))
		return get_var_size(g, e, gvar, lvar);

	printf("get_assignee_size e->code = %d\n", e->code);
	eet(e->tvar, "эээээээээ", 0);
	return 0;
}
