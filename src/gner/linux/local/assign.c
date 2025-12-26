#include "../../gner.h"
#include <stdio.h>

constr NOT_ASSIGNABLE = "Данное выражение не может быть присваеваемым.";

#define is_mov_mem_gvar_signature(r)                                           \
	(lceeu((r), AMPER) && lceep((r)->l, VAR) &&                                \
	 (gvar = find_glob_Var(g, (r)->l->tvar->view)))

int to_free_value_reg = 1;
struct Reg *assign_from_reg(Gg, struct LocalExpr *assignee, struct Reg *r1) {
	struct Reg *r2;
	struct LocalExpr *trailed;

	if (is_mem(assignee)) {
		if (is_xmm(r1)) {
			op_mem_(MOV_XMM, assignee, 0);
			reg_enter(r1);
		} else {
			op_mem_(MOV, assignee, 0);
			reg_enter(r1);
		}
	} else if ((trailed = is_not_assignable_or_trailed(assignee))) {
		struct BList *last_mem_str = 0;
		r2 = gen_to_reg_with_last_mem(g, assignee, trailed, &last_mem_str);
		r1 = get_reg_to_size(g, r1, unsafe_size_of_type(assignee->type));
		if (is_xmm(r1)) {
			op_last_mem_(MOV_XMM, last_mem_str); // not sure 'bout this
		} else {
			op_last_mem_(MOV, last_mem_str);
		}
		reg_enter(r1);

		free_register(r2);
	} else
		eet(assignee->tvar, NOT_ASSIGNABLE, 0);

	if (to_free_value_reg) {
		free_register(r1);
	}
	to_free_value_reg = 1;
	return r1;
}

void assign_from_literal(Gg, struct LocalExpr *assignee,
						 struct LocalExpr *literal, struct GlobVar *gvar) {
	struct Reg *r2;
	struct LocalExpr *trailed;

	if (is_mem(assignee)) {
		op_mem_(MOV, assignee, 0);
	} else if ((trailed = is_not_assignable_or_trailed(assignee))) {
		struct BList *last_mem_str = 0;
		r2 = gen_to_reg_with_last_mem(g, assignee, trailed, &last_mem_str);
		op_last_mem_(MOV, last_mem_str);
		free_register(r2);
	} else
		eet(assignee->tvar, NOT_ASSIGNABLE, 0);

	if (lceep(literal, INT)) {
		add_int_with_hex_comm(fun_text, literal->tvar->num);
	} else if (lceep(literal, REAL)) {
		real_add_enter(fun_text, literal->tvar->real);
	} else if (gvar) {
		blat_ft(gvar->signature), ft_add('\n');
	} else
		exit(55);
}

void tuple_to_tuple_assign(Gg, struct LocalExpr *e) {
	struct PList *assignee_tuple = e->l->tuple;
	struct PList *assignable_tuple = e->r->tuple;

	e->l->tuple = 0;
	plist_add(assignee_tuple, e->l);

	e->r->tuple = 0;
	plist_add(assignable_tuple, e->r);

	struct LocalExpr *l, *r, *sub_assign;
	struct PList *regs = new_plist(assignable_tuple->size);
	struct Reg *r1;
	u32 i;

	for (i = 0; i < assignable_tuple->size; i++) {
		l = plist_get(assignee_tuple, i);
		r = plist_get(assignable_tuple, i);

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
			assign_from_reg(g, l, r1);
		} else {
			r = plist_get(assignable_tuple, i);

			sub_assign = new_local_expr(LE_BIN_ASSIGN, 0, e->tvar);
			sub_assign->l = l, sub_assign->r = r;
			gen_assign(g, e);
			free(sub_assign);
		}
	}
	plist_free(regs);
}

int return_tuple_regs_indeces[MAX_ARGS_ON_REGISTERS] = {0, 1, 2, 3, 8, 9, 10};

void tuple_call_assign(Gg, struct LocalExpr *e) {
	gen_call(g, e->r);

	struct PList *returned_items_types = tup_itms(e->r->type);
	struct PList *regs = new_plist(returned_items_types->size);
	struct Reg *r1;
	struct PList *assignee_tuple = e->l->tuple;

	e->l->tuple = 0;
	plist_add(assignee_tuple, e->l);

	struct RegisterFamily *rf;
	struct TypeExpr *item_type;
	struct LocalExpr *assignee;
	u32 i;

	for (i = 0; i < returned_items_types->size; i++) {
		item_type = plist_get(returned_items_types, i);
		assignee = plist_get(assignee_tuple, i);

		if (is_le_num(assignee, 0)) {
			plist_add(regs, 0);
			continue;
		}

		int assignee_size = unsafe_size_of_type(assignee->type);
		int item_type_size = unsafe_size_of_type(item_type);
		rf = as_rfs(g->cpu)[return_tuple_regs_indeces[i]];

		if (!(r1 = alloc_reg_of_size(rf, item_type_size))) {
			r1 = try_borrow_reg(assignee->tvar, g, item_type_size);
			get_reg_to_rf(e->tvar, g, r1, rf);
		}
		plist_add(regs, get_reg_to_size(g, r1, assignee_size));
	}

	for (i = 0; i < assignee_tuple->size; i++) {
		assignee = plist_get(assignee_tuple, i);
		if ((r1 = plist_get(regs, i)))
			assign_from_reg(g, assignee, r1);
	}
	plist_free(regs);
}

void gen_assign(struct Gner *g, struct LocalExpr *e) {
	struct LocalExpr *assignee = e->l;
	struct GlobVar *gvar;

	if (assignee->tuple && e->r->tuple &&
		assignee->tuple->size == e->r->tuple->size && assignee->tuple->size > 0)
		tuple_to_tuple_assign(g, e);
	else if (e->r->type->code == TC_TUPLE)
		tuple_call_assign(g, e);
	else if ((is_num_le(e->r) || is_mov_mem_gvar_signature(e->r)) &&
			 !(lceep(e->r, REAL) && (is_sd(e->r->type) || is_sd(e->l->type))))
		assign_from_literal(g, assignee, e->r, gvar);
	else
		assign_from_reg(
			g, assignee,
			gen_to_reg(g, e->r, unsafe_size_of_type(assignee->type)));
}

struct Reg *assign_to_reg(Gg, struct LocalExpr *e) {
	to_free_value_reg = 0;
	return assign_from_reg(
		g, e->l, gen_to_reg(g, e->r, unsafe_size_of_type(e->l->type)));
}
