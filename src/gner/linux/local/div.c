#include "../../gner.h"
#include <stdio.h>

struct Reg *div_on_int(Gg, struct LocalExpr *e) {}

struct Reg *div_on_mem(Gg, struct LocalExpr *e) {}

struct Reg *div_on_reg(Gg, struct LocalExpr *e, struct Reg *r1,
					   struct Reg *r2) {
	struct Reg *rDX = 0;

	if (!is_rAX(r1->reg_code)) {
		if (g->cpu->a->r->allocated) {
			// r1 is now points to rAX
			swap_basic_regs(g, g->cpu->a, r1->rf, DO_XCHG);
		} else {
			mov_reg_(g, R_RAX);
			reg_enter(r1->rf->r->reg_code);
			free_reg_family(r1->rf);
			r1 = try_alloc_reg(e->tvar, g->cpu->a, r1->size);
		}
	}

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
