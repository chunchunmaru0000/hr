#include "../../gner.h"

// local var, arr(not ptr), field(not ptr)
// global var, arr(not ptr), field(not ptr)

struct Reg *gen_to_reg(Gg, struct LocalExpr *e, uc of_size) {
	int reg_size = of_size ? of_size : unsafe_size_of_type(e->type);
	struct Reg *res_reg = try_borrow_reg(e->tvar, g->cpu, reg_size);

	declare_lvar_gvar;

	if (lcep(VAR)) {
		get_assignee_size(g, e, &gvar, &lvar);
		mov_reg_var(g, res_reg->reg_code, lvar, gvar);
	} else
		exit(145);

	return res_reg;
}
