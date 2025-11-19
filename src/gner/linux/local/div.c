#include "../../gner.h"
#include <stdio.h>

void get_reg_to_rax(struct Token *tvar, Gg, struct Reg *reg) {
	if (!is_rAX(reg->reg_code)) {
		if (g->cpu->a->r->allocated) {
			// reg is now points to rAX
			swap_basic_regs(g, g->cpu->a, reg->rf, DO_XCHG);
		} else {
			mov_reg_(g, R_RAX);
			reg_enter(reg->rf->r->reg_code);
			free_reg_family(reg->rf);
			reg = try_alloc_reg(tvar, g->cpu->a, reg->size);
		}
	}
}

struct Reg *div_on_int(Gg, struct LocalExpr *e, struct Reg *r1,
					   struct LocalExpr *num) {
	exit(132);
}

struct Reg *div_on_mem(Gg, struct LocalExpr *e, struct Reg *r1,
					   struct LocalExpr *mem) {
	struct Reg *rDX = 0;

	declare_lvar_gvar;
	get_assignee_size(g, mem, &gvar, &lvar);

	get_reg_to_rax(e->tvar, g, r1);

	if (g->cpu->d->r->allocated) {
		rDX = try_borrow_reg(e->tvar, g, QWORD);
		swap_basic_regs(g, g->cpu->d, rDX->rf, DO_MOV); // rDX is rDX
	} else {
		rDX = try_alloc_reg(e->tvar, g->cpu->d, r1->size);
	}
	// CBW
	// CWDE
	// CDQE
	isprint_ft(IDIV);
	var_enter(lvar, gvar);
	// CWD
	// CDQ
	// CQO
	free_reg_family(rDX->rf);
	return r1;
}

struct Reg *div_on_reg(Gg, struct LocalExpr *e, struct Reg *r1,
					   struct Reg *r2) {
	struct Reg *rDX = 0;

	get_reg_to_rax(e->tvar, g, r1);

	if (is_rDX(r2->reg_code)) {
		rDX = try_borrow_reg(e->tvar, g, QWORD);
		swap_basic_regs(g, r2->rf, rDX->rf, DO_MOV); // r2 became not rDX
	} else if (g->cpu->d->r->allocated) {
		rDX = try_borrow_reg(e->tvar, g, QWORD);
		swap_basic_regs(g, g->cpu->d, rDX->rf, DO_MOV); // rDX is rDX
	} else {
		rDX = try_alloc_reg(e->tvar, g->cpu->d, r2->size);
	}

	// TODO: proper size
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
