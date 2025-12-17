#include "../../gner.h"
#include <stdio.h>

constr NOT_ASSIGNABLE = "Данное выражение не может быть присваеваемым.";

#define is_mov_mem_gvar_signature(r)                                           \
	(lceeu((r), AMPER) && lceep((r)->l, VAR) &&                                \
	 (gvar = find_glob_Var(g, (r)->l->tvar->view)))

void assign_to_mem(Gg, struct LocalExpr *e) {
	struct LocalExpr *mem = e->l, *assignable = e->r;
	int assignee_size = unsafe_size_of_type(mem->type);
	struct GlobVar *gvar;
	struct Reg *reg;

	gen_mem_tuple(g, mem);

	if (is_mov_mem_gvar_signature(assignable)) {
		op_mem_(MOV, mem, 0);
		blat_ft(gvar->signature), ft_add('\n');

	} else if (lceep(assignable, INT)) {
		gen_tuple_of(g, assignable);
		op_mem_(MOV, mem, 0);
		add_int_with_hex_comm(fun_text, assignable->tvar->num);

	} else if (lceep(assignable, REAL)) {
		// TODO: mov r, sd; mov mem, r
		gen_tuple_of(g, assignable);
		op_mem_(MOV, mem, 0);
		real_add_enter(fun_text, assignable->tvar->real);

	} else {
		reg = gen_to_reg(g, assignable, assignee_size);

		if (is_xmm(reg)) {
			op_mem_(MOV_XMM, mem, 0);
			reg_enter(reg->reg_code);
			free_reg(reg);
		} else {
			reg = get_reg_to_size(g, reg, assignee_size);
			op_mem_(MOV, mem, 0);
			reg_enter(reg->reg_code);
			free_reg_family(reg->rf);
		}
	}
}

void assign_to_last_mem(Gg, struct LocalExpr *assignee,
						struct LocalExpr *trailed, struct LocalExpr *right) {
	int assignee_size = unsafe_size_of_type(assignee->type);
	struct BList *last_mem_str = 0;
	struct Reg *r1 = 0, *r2 = 0;
	struct GlobVar *gvar;

	r1 = gen_to_reg_with_last_mem(g, assignee, trailed, &last_mem_str);

	if (is_mov_mem_gvar_signature(right)) {
		op_last_mem_(MOV, last_mem_str);
		blat_ft(gvar->signature), ft_add('\n');

	} else if (is_num_le(right)) {
		if (lceep(right, INT)) {
			op_last_mem_(MOV, last_mem_str);
			add_int_with_hex_comm(fun_text, right->tvar->num);
		} else if (is_ss(right->type)) {
			op_last_mem_(MOV, last_mem_str);
			real_add_enter(fun_text, right->tvar->real);
		} else {
			r2 = try_borrow_reg(right->tvar, g, QWORD);
			op_reg_(MOV, r2->reg_code);
			real_add_enter(fun_text, right->tvar->real);

			op_last_mem_(MOV, last_mem_str);
			reg_enter(r2->reg_code);
		}
	} else {
		r2 = gen_to_reg(g, right, assignee_size);
		if (!is_xmm(r2))
			r2 = get_reg_to_size(g, r2, assignee_size);
		op_last_mem_(MOV, last_mem_str);
		reg_enter(r2->reg_code);
	}

	free_reg_or_rf_if_not_zero(r1);
	free_reg_or_rf_if_not_zero(r2);
	blist_clear_free(last_mem_str);
}

void tuple_to_tuple_assign(Gg, struct LocalExpr *e) {
	struct PList *assignee_tuple = e->l->tuple;
	struct PList *assignable_tuple = e->r->tuple;

	struct LocalExpr *tmp;
	tmp = malloc(sizeof(*tmp));
	memcpy(tmp, e->l, sizeof(*tmp));
	tmp->tuple = 0;
	plist_add(assignee_tuple, tmp);

	tmp = malloc(sizeof(*tmp));
	memcpy(tmp, e->r, sizeof(*tmp));
	tmp->tuple = 0;
	plist_add(assignable_tuple, tmp);

	struct LocalExpr *l, *r, *sub_assign, *trailed;
	struct PList *regs = new_plist(assignable_tuple->size);
	struct Reg *r1, *r2;
	u32 i;

	for (i = 0; i < assignable_tuple->size; i++) {
		l = plist_get(assignee_tuple, i);
		define_le_type(l);
		r = plist_get(assignable_tuple, i);
		define_le_type(r);

		if (is_le_num(l, 0))
			gen_opted(g, r), plist_add(regs, 0);
		else
			plist_add(regs,
					  is_num_le(r)
						  ? 0
						  : gen_to_reg(g, r, unsafe_size_of_type(l->type)));
	}
	for (i = 0; i < assignee_tuple->size; i++) {
		l = plist_get(assignee_tuple, i);
		if (is_le_num(l, 0))
			continue;

		if ((r1 = plist_get(regs, i))) {
			if (is_mem(l)) {
				if (is_xmm(r1)) {
					op_mem_(MOV_XMM, l, 0);
					reg_enter(r1->reg_code);
					free_reg(r1);
				} else {
					op_mem_(MOV, l, 0);
					reg_enter(r1->reg_code);
					free_reg_family(r1->rf);
				}
			} else if ((trailed = is_not_assignable_or_trailed(l))) {
				struct BList *last_mem_str = 0;
				r2 = gen_to_reg_with_last_mem(g, l, trailed, &last_mem_str);
				r1 = get_reg_to_size(g, r1, unsafe_size_of_type(l->type));
				if (is_xmm(r1)) {
					op_last_mem_(MOV_XMM, last_mem_str); // not sure 'bout this
				} else {
					op_last_mem_(MOV, last_mem_str);
				}
				reg_enter(r1->reg_code);

				free_reg_or_rf_if_not_zero(r2);
				free_reg_or_rf_if_not_zero(r1);
			} else
				eet(e->l->tvar, NOT_ASSIGNABLE, 0);
		} else {
			r = plist_get(assignable_tuple, i);

			sub_assign = new_local_expr(LE_BIN_ASSIGN, 0, e->tvar);
			sub_assign->l = l, sub_assign->r = r;
			gen_assign(g, e);
			free(sub_assign);
		}
	}
}

void tuple_call_assign(Gg, struct LocalExpr *e) {}

void gen_assign(struct Gner *g, struct LocalExpr *e) {
	struct LocalExpr *assignee = e->l, *trailed;

	if (assignee->tuple && e->r->tuple &&
		assignee->tuple->size == e->r->tuple->size && assignee->tuple->size > 0)
		tuple_to_tuple_assign(g, e);
	else if (e->r->type->code == TC_TUPLE)
		tuple_call_assign(g, e);
	else if (is_mem(assignee))
		assign_to_mem(g, e);
	else if ((trailed = is_not_assignable_or_trailed(assignee)))
		assign_to_last_mem(g, assignee, trailed, e->r);
	else
		eet(e->l->tvar, NOT_ASSIGNABLE, 0);
}

struct Reg *assign_to_reg(Gg, struct LocalExpr *e) {
	struct LocalExpr *assignee = e->l, *trailed;
	int assignee_size = unsafe_size_of_type(assignee->type);
	struct BList *last_mem_str = 0;
	struct Reg *r1, *r2 = 0;

	r1 = gen_to_reg(g, e->r, 0);
	if (!is_xmm(r1))
		r1 = get_reg_to_size(g, r1, assignee_size);

	if (is_mem(assignee)) {
		gen_mem_tuple(g, assignee);

		if (is_xmm(r1)) {
			op_mem_(MOV_XMM, assignee, 0);
			reg_enter(r1->reg_code);
		} else {
			op_mem_(MOV, assignee, 0);
			reg_enter(r1->reg_code);
		}

	} else if ((trailed = is_not_assignable_or_trailed(assignee))) {
		r2 = gen_to_reg_with_last_mem(g, assignee, trailed, &last_mem_str);
		op_last_mem_(MOV, last_mem_str);
		reg_enter(r1->reg_code);
		// TODO: is xmm
	} else
		// TODO: else if is_le_num(assignee, 0)
		eet(e->l->tvar, NOT_ASSIGNABLE, 0);

	free_reg_or_rf_if_not_zero(r2);
	return r1;
}
