#include "../../gner.h"

struct Reg *div_on_int(Gg, struct LocalExpr *e) {}

struct Reg *div_on_mem(Gg, struct LocalExpr *e) {}

struct Reg *div_on_reg(Gg, struct LocalExpr *e, struct Reg *r1,
					   struct Reg *r2) {
	struct Reg *rDX = 0;

	if (!is_rAX(r1->reg_code))
		// r1 is now points to rAX
		swap_basic_regs(g, g->cpu->a, r1->rf, DO_XCHG);

	if (is_rDX(r2->reg_code)) {
		rDX = try_borrow_reg(e->tvar, g, QWORD);
		swap_basic_regs(g, r2->rf, rDX->rf, DO_MOV); // r2 became not rDX
	} else if (g->cpu->d->r->allocated) {
		rDX = try_borrow_reg(e->tvar, g, QWORD);
		swap_basic_regs(g, g->cpu->d, rDX->rf, DO_MOV); // rDX is rDX
	} else {
		rDX = try_alloc_reg(e->tvar, g->cpu->d, r2->size);
	}

 	// CBW
	// CWDE
	// CDQE
	isprint_ft(IDIV);
	reg_enter(r2->reg_code);
	// CWD
	// CDQ
	// CQO

	free_reg_family(r2->rf);
	free_reg_family(rDX->rf);
	return r1;
}
