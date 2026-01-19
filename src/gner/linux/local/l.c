#include "../../gner.h"
#include <stdio.h>

struct Reg *mem_to_reg(Gg, struct LocalExpr *e, int reg_size) {
	struct Reg *r;

	if (is_real_type(e->type)) {
		r = try_borrow_xmm_reg(e->tvar, g);
		mov_xmm_reg_(r);
		mem_enter(e, 0);
	} else {
		reg_size = unsafe_size_of_type(e->type);
		r = try_borrow_reg(e->tvar, g, unsafe_size_of_type(e->type));
		op_reg_(MOV, r);
		mem_enter(e, 0);
		if (reg_size)
			r = get_reg_to_size(g, r, reg_size);
	}
	return r;
}

struct Reg *assignable_to_reg(Gg, struct LocalExpr *e,
							  struct LocalExpr *trailed, int reg_size) {
	struct Reg *r, *xmm;
	struct BList *last_mem_str = 0;

	lm_size = reg_size;
	r = gen_to_reg_with_last_mem(g, e, trailed, &last_mem_str);
	if (is_real_type(e->type)) {
		xmm = try_borrow_xmm_reg(e->tvar, g);
		mov_xmm_reg_(xmm);

		free_register(r), r = xmm;
	} else {
		op_reg_(MOV, r);
	}
	last_mem_enter(last_mem_str);

	blist_clear_free(last_mem_str);
	return r;
}

struct LocalString *isnt_uniq_str(Gg, struct BList *str) {
	for (u32 i = 0; i < g->strs->size; i++) {
		struct LocalString *local_str = plist_get(g->strs, i);
		if (sc(bs(str), bs(local_str->str)))
			return local_str;
	}
	return 0;
}

struct Reg *prime_str(Gg, struct LocalExpr *e) {
	struct LocalString *local_str;
	struct BList *str = e->tvar->str;
	struct Reg *r = try_borrow_reg(e->tvar, g, QWORD);

	if (!(local_str = isnt_uniq_str(g, str))) {
		local_str = malloc(sizeof(struct LocalString));
		local_str->str = str;
		local_str->ptr_to_str = take_label(LC_STR);
		plist_add(g->strs, local_str);

		if (!g->flags->is_data_segment_used) {
			iprint_prol(SA_SEGMENT_READ_WRITE);
			g->flags->is_data_segment_used = 1;
		}

		blat_str_aprol(SA_LET);
		blat_aprol(local_str->ptr_to_str), aprol_add(' ');
		blat_str_aprol(SA_BYTE);
		blat_aprol(e->tvar->view);
		print_aprol(SA_ZERO_TERMINATOR);
	}

	op_reg_(MOV, r);
	blat_ft_enter(local_str->ptr_to_str);

	return r;
}

struct Reg *prime_to_reg(Gg, struct LocalExpr *e, int reg_size) {
	struct Reg *reg = 0, *xmm;
	int unit_size;

	if (e->code == LE_PRIMARY_VAR) {
		printf("use mem\n");
		exit(101);
	} else if (lcep(REAL)) {
		unit_size = e->type->code == TC_SINGLE ? DWORD : QWORD;

		reg = try_borrow_reg(e->tvar, g, unit_size);
		if (e->tvar->real) {
			op_reg_(MOV, reg);
			real_add_enter(fun_text, e->tvar->real);
		} else {
			op_reg_reg(XOR, reg, reg);
		}

		xmm = try_borrow_xmm_reg(e->tvar, g);
		mov_xmm_reg_(xmm);
		reg_enter(reg);

		free_register(reg);
		reg = xmm;

	} else if (lcep(INT)) {
		reg = try_borrow_reg(e->tvar, g, reg_size);

		if (e->tvar->num == 0) {
			op_reg_reg(XOR, reg, reg);
		} else if (e->tvar->num == 1 && reg->size > BYTE) {
			op_reg_reg(XOR, reg, reg);
			op_reg_enter(INC, reg);
		} else {
			op_reg_(MOV, reg);
			add_int_with_hex_comm(fun_text, e->tvar->num);
		}
	} else if (lcep(STR)) {
		reg = prime_str(g, e);
	} else
		exit(145);
	return reg;
}

struct Reg *dereference(Gg, struct LocalExpr *e) {
	struct Reg *r = 0;
	struct LocalExpr *trailed;

	if (is_mem(e)) {
		r = try_borrow_reg(e->tvar, g, QWORD);
		gen_mem_tuple(g, e);
		op_reg_(LEA, r);
		mem_enter(e, QWORD);
	} else if ((trailed = is_not_assignable_or_trailed(e))) {
		struct BList *last_mem_str = 0;
		lm_size = QWORD;
		r = gen_to_reg_with_last_mem(g, e, trailed, &last_mem_str);
		op_reg_(LEA, r);
		last_mem_enter(last_mem_str);
		blist_clear_free(last_mem_str);
	}

	if (r == 0)
		exit(159);
	return r;
}

constr CANT_CAST_PTR_TO_SINGLE =
	"Нельзя 'окак' выражение типа 'в32' в указатель.";

struct Reg *cast_reg_to_type(Gg, struct TypeExpr *cast_type,
							 struct LocalExpr *cast_value, struct Reg *r) {
	if (is_ptr_type(cast_type)) {
		if (is_xmm(r)) {
			if (is_ss(cast_value->type))
				eet(cast_value->tvar, CANT_CAST_PTR_TO_SINGLE, 0);

			struct Reg *r2 = try_borrow_reg(cast_value->tvar, g, QWORD);
			op_reg_reg(MOV_XMM, r2, r);
			free_register(r), r = r2;
		} else
			r = get_reg_to_size(g, r, QWORD);
	} else if (is_num_int_type(cast_type)) {
		r = is_xmm(r) ? get_reg_to_size(g, cvt_from_xmm(g, cast_value, r),
										unsafe_size_of_type(cast_type))
					  : get_reg_to_size(g, r, unsafe_size_of_type(cast_type));
	} else if (is_real_type(cast_type)) {
		if (!is_xmm(r))
			r = cvt_to_xmm(g, cast_value, r, is_ss(cast_type));
		else if (cast_type->code > cast_value->type->code) // to DOUBLE
			cvt_ss_to_sd(r);
		else if (cast_type->code < cast_value->type->code) { // to SINGLE
			op_reg_reg(CVTSD2SS, r, r);
		}
	} else
		exit(44);

	return r;
}

struct Reg *cast_to_type(Gg, struct TypeExpr *cast_type,
						 struct LocalExpr *cast_value) {
	return cast_reg_to_type(g, cast_type, cast_value,
							gen_to_reg(g, cast_value, 0));
}

void *var_os_data = 0;
struct PList *some_os = &(struct PList){&var_os_data, 1, 1, 1};

void delcare_var_from_LE_DECLARE_VAR(Gg, struct LocalExpr *data) {
	struct Arg *arg = new_arg(); // offset gonna be 0
	plist_add(arg->names, data->l->tvar);
	arg->type = data->type;
	arg->arg_size = unsafe_size_of_type(arg->type);

	some_os->st[0] = arg;
	put_vars_on_the_stack(g, some_os);

	// erase declare_var to just var
	paste_le(data, data->l);
}

struct Reg *le_declare_var(Gg, struct LocalExpr *data) {
	// declare var
	delcare_var_from_LE_DECLARE_VAR(g, data);
	// eval var
	return mem_to_reg(g, data, 0);
}

constr SIZES_LITERALLY_DIFFER =
	"Размеры указанного типа и выражения 'буквально' отличаются.";

struct Reg *literally_to_reg(Gg, struct TypeExpr *literally_type,
							 struct LocalExpr *value) {
	int literally_size = unsafe_size_of_type(literally_type);

	if (literally_size != unsafe_size_of_type(value->type))
		eet(value->tvar, SIZES_LITERALLY_DIFFER, 0);

	struct Reg *r = gen_to_reg(g, value, 0), *r2;

	if (is_real_type(literally_type)) {
		if (!is_xmm(r)) {
			r2 = try_borrow_xmm_reg(value->tvar, g);
			op_reg_reg(MOV_XMM, r2, r);
			free_register(r), r = r2;
		}
	} else {
		if (is_xmm(r)) {
			r2 = try_borrow_reg(value->tvar, g, literally_size);
			op_reg_reg(MOV_XMM, r2, r);
			free_register(r), r = r2;
		}
	}

	return r;
}

struct Reg *unary_to_reg(Gg, struct LocalExpr *e) {
	struct Reg *reg = 0;
	int unit_size;

	if (lceu(MINUS)) {
		reg = gen_to_reg(g, e->l, 0);
		op_reg_enter(NEG, reg);
	} else if (lceu(INC) || lceu(DEC)) {
		reg = unary_dec_inc(g, e->l, lceu(INC));
	} else if (lceu(NOT) || lce(BOOL)) {
		cmp_bool(g, e->l);
		reg = try_borrow_reg(e->tvar, g, BYTE);
		if (lce(BOOL))
			op_(SETN0);
		else
			op_(SET0);
		reg_enter(reg);
	} else if (lceu(BIT_NOT)) {
		reg = gen_to_reg(g, e->l, 0);
		op_reg_enter(NOT, reg);
	} else if (lceu(AMPER)) {
		reg = dereference(g, e->l);
	} else if (lceu(ADDR)) {
		unit_size = unsafe_size_of_type(e->type);
		reg = gen_to_reg(g, e->l, 0); // QWORD by itself
		op_reg_(MOV, reg);
		sib(g, unit_size, 0, 0, reg->reg_code, 0, 0), ft_add('\n');
	}

	if (reg == 0)
		exit(158);
	return reg;
}

struct Reg *gen_to_reg(Gg, struct LocalExpr *e, uc of_size) {
	int reg_size = of_size ? of_size : unsafe_size_of_type(e->type);
	struct LocalExpr *trailed;
	struct Reg *res_reg;

	if (is_mem(e)) {
		gen_mem_tuple(g, e);
		return mem_to_reg(g, e, reg_size);
	}
	if ((trailed = is_not_assignable_or_trailed(e))) {
		// TODO: gen_assignable_tuple
		return assignable_to_reg(g, e, trailed, reg_size);
	}

	// TODO: use gen_tuple_of whenever is need
	gen_tuple_of(g, e);

	if (is_primary(e))
		res_reg = prime_to_reg(g, e, reg_size);
	else if (is_unary(e) || lce(BOOL))
		res_reg = unary_to_reg(g, e);
	else if (lce(AS))
		res_reg = cast_to_type(g, e->type, e->l);
	else if (lce(LITERALLY))
		res_reg = literally_to_reg(g, e->type, e->l);
	else if (lce(DECLARE_VAR))
		res_reg = le_declare_var(g, e);
	else if (lcea(CALL))
		res_reg = call_to_reg(g, e);
	else if (lcea(INC) || lcea(DEC))
		res_reg = after_dec_inc(g, e->l, lcea(INC));
	else if (lceb(AND))
		res_reg = and_to_reg(g, e);
	else if (lceb(OR))
		res_reg = or_to_reg(g, e);
	else if (is_uses_cmp(e))
		return cmp_with_set(g, e);
	else if (is_bin_le(e))
		res_reg = bin_to_reg(g, e);
	else if (lceb(TERRY))
		res_reg = terry_to_reg(g, e);
	else if (lceb(ASSIGN))
		res_reg = assign_to_reg(g, e);
	else
		exit(152);
	return is_xmm(res_reg) ? res_reg : get_reg_to_size(g, res_reg, reg_size);
}
