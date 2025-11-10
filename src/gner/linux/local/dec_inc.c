#include "../../gner.h"

void add_var_(struct Gner *g, let_lvar_gvar) {
	iprint_fun_text(SA_ADD); // плюс
	var_(g, lvar, gvar);
}

void gen_inc(struct Gner *g, struct LocalExpr *inced) {
	struct GlobVar *gvar = 0;
	struct LocalVar *lvar = 0;

	if (lceep(inced, VAR)) {
		get_assignee_size(g, inced, &gvar, &lvar);
		add_var_(g, lvar, gvar);
		add_int_with_hex_comm(fun_text, 1);
	}
}

void sub_var_(struct Gner *g, let_lvar_gvar) {
	iprint_fun_text(SA_SUB); // минс
	var_(g, lvar, gvar);
}

void gen_dec(struct Gner *g, struct LocalExpr *deced) {
	struct GlobVar *gvar = 0;
	struct LocalVar *lvar = 0;

	if (lceep(deced, VAR)) {
		get_assignee_size(g, deced, &gvar, &lvar);
		sub_var_(g, lvar, gvar);
		add_int_with_hex_comm(fun_text, 1);
	}
}
