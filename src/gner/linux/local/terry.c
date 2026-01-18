#include "../../gner.h"
#include <stdio.h>

struct Reg *terry_to_reg(Gg, struct LocalExpr *e) {
	struct Reg *r1 = 0, *r2 = 0;
	int res_size = unsafe_size_of_type(e->type);

	struct BList *r_result_label, *exit_label;

	r_result_label = take_label(LC_ELSE);
	exit_label = take_label(LC_ELSE);

	and_cmp(g, e->co.cond, r_result_label);

	// l
	r1 = gen_to_reg(g, e->l, res_size);
	free_register(r1);
	// jmp exit
	jmp_(exit_label);
	// r
	add_label(r_result_label);
	r2 = gen_to_reg(g, e->r, res_size);
	get_reg_to_rf(e->tvar, g, r2, r1->rf);
	// exit_label:
	add_label(exit_label);

	blist_clear_free(r_result_label);
	blist_clear_free(exit_label);

	if (!r2)
		exit(193);
	return r2;
}

void gen_terry(Gg, struct LocalExpr *e) {
	struct BList *r_result_label = take_label(LC_ELSE), *exit_label;

	if (causes_more_than_just_gvar(e->l) && causes_more_than_just_gvar(e->r)) {
		exit_label = take_label(LC_ELSE);
		and_cmp(g, e->co.cond, r_result_label);

		// l
		gen_local_expr_linux(g, e->l);
		jmp_(exit_label);
		// r
		add_label(r_result_label);
		gen_local_expr_linux(g, e->r);
		// exit_label:
		add_label(exit_label);
		blist_clear_free(exit_label);

	} else if (causes_more_than_just_gvar(e->l)) {
		and_cmp(g, e->co.cond, r_result_label);
		// l
		gen_local_expr_linux(g, e->l);
		// r
		add_label(r_result_label);

	} else if (causes_more_than_just_gvar(e->r)) {
		or_cmp(g, e->co.cond, r_result_label);
		// r
		gen_local_expr_linux(g, e->r);
		add_label(r_result_label);

	} else
		exit(190);

	blist_clear_free(r_result_label);
}

void gen_if_else(Gg, struct LocalExpr *e) {
	struct BList *r_label = take_label(LC_ELSE),
				 *exit_label = take_label(LC_ELSE);

	and_cmp(g, e->co.cond, r_label);
	// l
	gen_block(g, (void *)e->l);
	plist_free((void *)e->l);
	jmp_(exit_label);
	// r
	add_label(r_label);
	gen_block(g, (void *)e->r);
	plist_free((void *)e->r);
	// exit_label:
	add_label(exit_label);

	blist_clear_free(exit_label);
	blist_clear_free(r_label);
}

void gen_if(struct Gner *g, struct LocalExpr *e) {
	struct BList *exit_label = take_label(LC_ELSE);

	and_cmp(g, e->co.cond, exit_label);
	// l
	gen_block(g, (void *)e->l);
	plist_free((void *)e->l);
	// exit_label:
	add_label(exit_label);

	blist_clear_free(exit_label);
}

void gen_if_elif(struct Gner *g, struct LocalExpr *e) {
	struct BList *r_label = take_label(LC_ELSE),
				 *exit_label = take_label(LC_ELSE);

	and_cmp(g, e->co.cond, r_label);
	// l
	gen_block(g, (void *)e->l);
	plist_free((void *)e->l);
	jmp_(exit_label);
	// r
	add_label(r_label);
	gen_local_expr_linux(g, e->r);
	// exit_label:
	add_label(exit_label);

	blist_clear_free(exit_label);
	blist_clear_free(r_label);
}

#define is_back ((e->flags & LOOP_IS_BACKWARD))

void gen_range_loop(struct Gner *g, struct LocalExpr *e) {
	struct BList *block_begin = take_label(LC_ELSE),
				 *cmp_begin = take_label(LC_ELSE);
	struct LocalExpr *other;
	struct Loop *loop = (void *)e->tvar->str; // e->tvar->str is loop
	// assignable_e => (e0[...|..=]e1 [шаг|шагом e2] [назад]) ( ... )

	// first thing is assignable_e = e0
	other = new_local_expr(LE_BIN_ASSIGN, 0, e->tvar);
	other->l = (void *)e->tvar->num; // e->tvar->num is assignable_e
	other->r = e->l;				 // e->l is e0
	gen_local_expr_linux(g, other), free(other);

	// then jmp to cmp
	jmp_(cmp_begin);
	// next is just block
	add_label(block_begin);
	gen_block(g, (void *)(long)e->tvar->real); // e->tvar->real is block

	// inc/dec and continue label
	if (loop->cont) {
		add_label(loop->cont);
		blist_clear_free(loop->cont);
	}
	if (!e->co.cond) {				  // e->co.cond is e2
		other = (void *)e->tvar->num; // e->tvar->num is assignable_e
		gen_dec_inc(g, other, !is_back);
	} else {
		exit(99); // TODO: make step
	}

	// compare
	add_label(cmp_begin);
	other = new_local_expr(0, 0, e->tvar);
	other->code = is_back ? (e->flags & LOOP_DDE) ? LE_BIN_MOREE : LE_BIN_MORE
				  : (e->flags & LOOP_DDE) ? LE_BIN_LESSE
										  : LE_BIN_LESS;
	other->l = (void *)e->tvar->num; // e->tvar->num is assignable_e
	other->r = e->r;				 // e->r is e1
	just_cmp(g, other);
	iprint_jmp(g, other->code, is_u_type(other->l->type->code));
	free(other);
	blat_ft_enter(block_begin);

	// break label
	if (loop->brek) {
		add_label(loop->brek);
		blist_clear_free(loop->brek);
	}
	free(loop);
	blist_clear_free(cmp_begin);
	blist_clear_free(block_begin);
}
