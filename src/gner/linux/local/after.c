#include "../../gner.h"
#include <stdio.h>

struct Reg *index_to_reg(Gg, struct LocalExpr *e, int reg_size) {
	;
	return 0;
}

struct Reg *inc_to_reg(Gg, struct LocalExpr *e, int reg_size) {
	;
	return 0;
}

struct Reg *dec_to_reg(Gg, struct LocalExpr *e, int reg_size) {
	;
	return 0;
}

// for field_of_ptr_to_reg and field_to_reg
u8 during_add = 0;
u8 during_of_ptr_add = 0;

struct Reg *field_of_ptr_to_reg(Gg, struct LocalExpr *e, int reg_size) {
	struct Reg *r1;
	struct Arg *arg = (struct Arg *)e->tvar->num;
	struct BList *field_full_name = (struct BList *)(long)e->tvar->real;

	if (!during_add && !during_of_ptr_add)
		reg_size = arg->arg_size;

	r1 = gen_to_reg(g, e->l, QWORD);
	op_reg_(MOV, r1->reg_code);
	sib_enter(reg_size, 0, 0, r1->reg_code, field_full_name, 1);

	if (during_add)
		g->fun_text->size -= 2; // remove ")\n" after sib_enter

	blist_clear_free(field_full_name);
	return r1;
}
struct Reg *field_to_reg(Gg, struct LocalExpr *e, int reg_size) {
	struct Reg *r1;
	struct Arg *arg = (struct Arg *)e->tvar->num;
	struct BList *field_full_name = (struct BList *)(long)e->tvar->real;
	enum RegCode base;

	printf("[%s] during_add = %d of size %d\n", bs(field_full_name), during_add,
		   reg_size);
	if (lceep(e->l, VAR)) {
		declare_lvar_gvar;
		get_assignee_size(g, e->l, &gvar, &lvar);

		blist_add(field_full_name, '+');
		lvar ? (blat_blist(field_full_name, lvar->name->view), base = R_RBP)
			 : (blat_blist(field_full_name, gvar->signature), base = 0);

		if (!during_add)
			reg_size = arg->arg_size;
		printf("[var] during_add = %d of size %d\n", during_add, reg_size);
		r1 = try_borrow_reg(e->tvar, g, reg_size);
		op_reg_(MOV, r1->reg_code);
		sib_enter(reg_size, 0, 0, base, field_full_name, 1);

		if (during_add)
			g->fun_text->size -= 2; // remove ")\n" after sib_enter

	} else if (lceeu(e->l, ADDR)) {
		r1 = gen_to_reg(g, e->l->l, QWORD);
		op_reg_(MOV, r1->reg_code);
		sib_enter(arg->arg_size, 0, 0, r1->reg_code, field_full_name, 1);

		if (during_add)
			g->fun_text->size -= 2; // remove ")\n" after sib_enter

	} else if (lceea(e->l, FIELD_OF_PTR)) {
		if (during_add) {
			r1 = field_of_ptr_to_reg(g, e->l, reg_size);
			ft_add('+');
			blat_ft(field_full_name);
		} else {
			printf("begin FIELD_OF_PTR of size %d\n", arg->arg_size);
			during_of_ptr_add = 1;
			r1 = field_of_ptr_to_reg(g, e->l, arg->arg_size);
			during_of_ptr_add = 0;

			g->fun_text->size -= 2; // remove ")\n" after sib_enter
			ft_add('+');
			blat_ft(field_full_name);
			ft_add(')'), ft_add('\n');
		}
	} else if (lceea(e->l, FIELD)) {
		printf("[%s] during_add = %d of size %d\n", bs(field_full_name),
			   during_add, reg_size);
		if (during_add) {
			r1 = field_to_reg(g, e->l, reg_size);
			ft_add('+');
			blat_ft(field_full_name);
		} else {
			during_add = 1;
			printf("begin FIELD during_add of size %d\n", arg->arg_size);
			r1 = field_to_reg(g, e->l, arg->arg_size);
			during_add = 0;

			ft_add('+');
			blat_ft(field_full_name);
			ft_add(')'), ft_add('\n');
		}
	} else {
		printf("[%d]\n", e->l->code);
		eet(e->tvar, "эээ че type 0, где define_type", 0);
	}

	blist_clear_free(field_full_name);
	return r1;
}

struct Reg *after_to_reg(Gg, struct LocalExpr *e, int reg_size) {
	struct Reg *r1 = 0;

	if (lcea(INDEX)) {
	} else if (lcea(CALL)) {
	} else if (lcea(INC)) {
	} else if (lcea(DEC)) {
	} else if (lcea(FIELD_OF_PTR))
		r1 = field_of_ptr_to_reg(g, e, reg_size);
	else if (lcea(FIELD))
		r1 = field_to_reg(g, e, reg_size);

	if (!r1)
		exit(146);
	return r1;
}
