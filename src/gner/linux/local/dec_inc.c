#include "../../gner.h"
#include <stdio.h>

#define Lvar struct LocalExpr *var
#define Lv struct LocalVar *
#define Gv struct GlobVar *
#define Le struct LocalExpr *
#define LeE struct LocalExpr *e
#define U long unit
#define inced uc is_inc

constr ALLOWANCE_OF_INDEXATION =
	"Индексация разрешена только по массивам и указателям.";

#define let_lvar_gvar_inc let_lvar_gvar, uc is_inc
#define add_or_sub                                                             \
	do {                                                                       \
		if (is_inc)                                                            \
			iprint_fun_text(SA_ADD);                                           \
		else                                                                   \
			iprint_fun_text(SA_SUB);                                           \
	} while (0)

long add_of_type(struct Token *tvar, struct TypeExpr *type) {
	if (is_int_type(type))
		return 1;
	if (type->code == TC_PTR)
		return unsafe_size_of_type(type->data.ptr_target);

	eet(tvar, "Нельзя '++' или '--' сюда.", 0);
	return 0;
}

// var[++ / --]
void var_indec(Gg, Lvar, uc is_inc) {
	declare_lvar_gvar;
	get_assignee_size(g, var, &gvar, &lvar);

	add_or_sub;
	var_(g, lvar, gvar);
	add_int_with_hex_comm(fun_text, add_of_type(var->tvar, lvar_gvar_type()));
}

// (*var)[++ / --]
void var_ptr_indec(Gg, Lvar, uc is_inc) {
	struct TypeExpr *unit_type = ptr_targ(var->type);
	int unit = add_of_type(var->tvar, unit_type);
	int unit_size = unsafe_size_of_type(unit_type);

	struct Reg *reg = gen_to_reg(g, var, QWORD);

	add_or_sub;
	sib_(unit_size, 0, 0, reg->reg_code, 0, 0);
	add_int_with_hex_comm(fun_text, unit);
	free_reg_family(reg->rf);
}

Le try_add_index_summ_to_disp(struct LocalExpr *index, long unit_size,
							  struct BList *disp) {
	struct LocalExpr *num;
	struct BList *tmp_str;

	if (lceeb(index, ADD) &&
		(lceep(index->l, INT)	? (num = index->l, index = index->r)
		 : lceep(index->r, INT) ? (num = index->r, index = index->l)
								: 0)) {
		blist_add(disp, ' ');
		blist_add(disp, '+');
		blist_add(disp, ' ');
		tmp_str = int_to_str(unit_size * num->tvar->num);
		copy_to_fst_and_clear_snd(disp, tmp_str);
	}
	return index;
}
Le try_find_index_summ_as_disp(struct LocalExpr *index, long unit_size,
							   long *disp) {
	struct LocalExpr *num;

	if (lceeb(index, ADD) &&
		(lceep(index->l, INT)	? (num = index->l, index = index->r)
		 : lceep(index->r, INT) ? (num = index->r, index = index->l)
								: 0))
		*disp = unit_size * num->tvar->num;
	else
		*disp = 0;
	return index;
}

// var[var or int][++ / --]
void var_index_indec(Gg, struct LocalExpr *e, uc is_inc) {
	Lvar = e->l;
	struct LocalExpr *index = e->r;
	struct Reg *index_reg = 0, *var_reg = 0;
	declare_lvar_gvar;
	get_assignee_size(g, var, &gvar, &lvar);

	struct TypeExpr *unit_type;
	long unit;
	int item_size;
	struct BList *disp_str;
	long disp_num = 0;
	enum RegCode base;

	if (var->type->code == TC_ARR) {
		unit_type = arr_type(var->type);
		unit = add_of_type(var->tvar, unit_type);

		item_size = unsafe_size_of_type(unit_type);
		base = lvar ? R_RBP : 0;

		if (lceep(index, INT)) {
			gen_tuple_of(g, index);
			disp_str = int_to_str(index->tvar->num * item_size);
			blist_add(disp_str, '+');
			blat_blist(disp_str, gvar ? gvar->signature : lvar->name->view);
			add_or_sub;
			sib_(item_size, base, 0, 0, (long)disp_str, 1);

		} else {
			disp_str = copy_str(lvar ? lvar->name->view : gvar->signature);
			index = try_add_index_summ_to_disp(index, item_size, disp_str);

			index_reg = gen_to_reg(g, index, QWORD);
			add_or_sub;
			sib_(item_size, base, item_size, index_reg->rf->r->reg_code,
				 (long)disp_str, 1);
		}
		blist_clear_free(disp_str);

	} else if (var->type->code == TC_PTR) {
		unit_type = ptr_targ(var->type);
		unit = add_of_type(var->tvar, unit_type);

		item_size = unsafe_size_of_type(unit_type);
		base = lvar ? R_RBP : 0;

		if (lceep(index, INT)) {
			gen_tuple_of(g, index);
			var_reg = gen_to_reg(g, var, QWORD);
			add_or_sub;
			sib_(item_size, var_reg->reg_code, 0, 0,
				 index->tvar->num * item_size, 0);

		} else {
			if ((var_reg = borrow_basic_reg(g->cpu, QWORD))) {
				mov_reg_var(g, var_reg->reg_code, lvar, gvar);
				index =
					try_find_index_summ_as_disp(index, item_size, &disp_num);
				index_reg = gen_to_reg(g, index, QWORD);
				add_or_sub;
				sib_(item_size, var_reg->reg_code, item_size,
					 index_reg->reg_code, disp_num, 0);
			} else {
				disp_str = copy_str(lvar ? lvar->name->view : gvar->signature);
				index = try_add_index_summ_to_disp(index, item_size, disp_str);

				index_reg = gen_to_reg(g, index, QWORD);

				// lea 	   rax, qword [vsize rax] 	// rax is index
				op_reg_(LEA, index_reg->reg_code);
				sib(g, QWORD, 0, item_size, index_reg->reg_code, 0, 0),
					ft_add('\n');
				// add     rax, qword [rbp var] 	// qword cuz index from ptr
				op_reg_(ADD, index_reg->reg_code);
				sib(g, QWORD, base, 0, 0, (long)disp_str, 1), ft_add('\n');
				// add     item_size [rax], int
				add_or_sub;
				sib_(item_size, index_reg->reg_code, 0, 0, 0, 0);

				blist_clear_free(disp_str);
			}
		}
	} else
		eet(e->tvar, ALLOWANCE_OF_INDEXATION, 0);

	add_int_with_hex_comm(fun_text, unit);

	if (var_reg)
		free_reg_family(var_reg->rf);
	if (index_reg)
		free_reg_family(index_reg->rf);
}

// var-[> / @]field[++ / --]
void var_field_indec(Gg, struct LocalExpr *e, uc is_inc) {
	// from define_struct_field_type_type
	struct Arg *feld = (struct Arg *)e->tvar->num;
	struct BList *disp = (struct BList *)(long)e->tvar->real;

	struct LocalExpr *var = e->l;
	struct Reg *var_reg = 0;

	struct TypeExpr *unit_type = feld->type;
	long unit = add_of_type(var->tvar, unit_type);
	int unit_size = feld->arg_size;
	enum RegCode base;

	if (lcea(FIELD)) {
		declare_lvar_gvar;
		get_assignee_size(g, var, &gvar, &lvar);
		blist_add(disp, ' '), blist_add(disp, '+'), blist_add(disp, ' ');

		lvar ? (blat_blist(disp, lvar->name->view), base = R_RBP)
			 : (blat_blist(disp, gvar->signature), base = 0);
	} else {
		// add     unit_size [rax + offset], unit
		base = (var_reg = gen_to_reg(g, var, QWORD))->reg_code;
	}

	add_or_sub;
	sib_(unit_size, 0, 0, base, disp, 1);
	add_int_with_hex_comm(fun_text, unit);

	blist_clear_free(disp);
	if (var_reg)
		free_reg_family(var_reg->rf);
}

void any_indec(Gg, struct LocalExpr *e, uc is_inc) {
	// addr = gen_left_side_addr(g, inced);
	add_or_sub;
	// blat_fun_text(inced_addr->name);
	fun_text_add(' ');
	add_int_with_hex_comm(fun_text, 1);
	fun_text_add('\n');
}

void gen_dec_inc(Gg, struct LocalExpr *e, uc is_inc) {
	if (lcep(VAR)) {
		// var[++ / --]
		var_indec(g, e, is_inc);

	} else if (lceu(ADDR) && lceep(e->l, VAR)) {
		// *var[++ / --]
		var_ptr_indec(g, e->l, is_inc);

	} else if (lcea(INDEX) && lceep(e->l, VAR)) {
		// var[var or int][++ / --]
		var_index_indec(g, e, is_inc);

	} else if ((lcea(FIELD_OF_PTR) || lcea(FIELD)) && lceep(e->l, VAR)) {
		// var-[> / @]field[++ / --]
		var_field_indec(g, e, is_inc);

	} else
		any_indec(g, e, is_inc);
}
