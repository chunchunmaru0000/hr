#include "../../gner.h"
#include <stdio.h>

#define cbe(bin) (code == LE_BIN_##bin)
#define str_len_be(code) ((str = SA_##code, len = SA_##code##_LEN))

void iprint_op(Gg, enum LE_Code code) {
	const char *str;
	u32 len;
	cbe(MUL)	   ? str_len_be(IMUL)
	: cbe(ADD)	   ? str_len_be(ADD)
	: cbe(SUB)	   ? str_len_be(SUB)
	: cbe(BIT_AND) ? str_len_be(BIT_AND)
	: cbe(BIT_XOR) ? str_len_be(BIT_XOR)
	: cbe(BIT_OR)  ? str_len_be(BIT_OR)
				   : exit(155);
	indent_line(g, g->fun_text);
	blat(g->fun_text, (uc *)str, len - 1);
}

// cmpeqss/cmpeqsd    ; равно ==
// cmpneqss/cmpneqsd  ; не равно !=
// cmpltss/cmpltsd    ; меньше <
// cmpless/cmplesd    ; меньше или равно <=
// cmpgtss/cmpgtsd    ; больше >
// cmpgess/cmpgesd    ; больше или равно >=

// maxss/maxsd    ; максимум
// minss/minsd    ; минимум
// sqrtss/sqrtsd   ; квадратный корень
void iprint_xmm_op(Gg, enum LE_Code code, uc is_ss) {
	const char *str;
	u32 len;

	if (is_ss) {
		cbe(MUL)	   ? str_len_be(MUL_SS)
		: cbe(DIV)	   ? str_len_be(DIV_SS)
		: cbe(ADD)	   ? str_len_be(ADD_SS)
		: cbe(SUB)	   ? str_len_be(SUB_SS)
		: cbe(BIT_AND) ? str_len_be(BIT_AND_PS)
		: cbe(BIT_XOR) ? str_len_be(BIT_XOR_PS)
		: cbe(BIT_OR)  ? str_len_be(BIT_OR_PS)
					   : exit(162);
	} else {
		cbe(MUL)	   ? str_len_be(MUL_SD)
		: cbe(DIV)	   ? str_len_be(DIV_SD)
		: cbe(ADD)	   ? str_len_be(ADD_SD)
		: cbe(SUB)	   ? str_len_be(SUB_SD)
		: cbe(BIT_AND) ? str_len_be(BIT_AND_PD)
		: cbe(BIT_XOR) ? str_len_be(BIT_XOR_PD)
		: cbe(BIT_OR)  ? str_len_be(BIT_OR_PD)
					   : exit(163);
	}
	indent_line(g, g->fun_text);
	blat(g->fun_text, (uc *)str, len - 1);
}

struct Reg *mem_to_reg(Gg, struct LocalExpr *e, int reg_size) {
	struct Reg *r;

	if (is_real_type(e->type)) {
		r = try_borrow_xmm_reg(e->tvar, g);
		mov_xmm_reg_(r);
	} else {
		if (!reg_size)
			reg_size = unsafe_size_of_type(e->type);
		r = try_borrow_reg(e->tvar, g, reg_size);
		op_reg_(MOV, r);
	}
	mem_enter(e, 0);

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

struct Reg *cvt_from_xmm(Gg, struct LocalExpr *e, struct Reg *xmm_reg) {
	struct Reg *r;

	if (is_ss(e->type)) {
		op_(CVTSS2SI);
		r = try_borrow_reg(e->tvar, g, DWORD);
	} else {
		op_(CVTSD2SI);
		r = try_borrow_reg(e->tvar, g, QWORD);
	}
	reg_(r);
	reg_enter(xmm_reg);
	free_reg(xmm_reg);
	return r;
}

#define ss_or_sd                                                               \
	if (to_ss)                                                                 \
		op_(CVTSI2SS);                                                         \
	else                                                                       \
		op_(CVTSI2SD);

// TODO: pxor
struct Reg *cvt_to_xmm(Gg, struct LocalExpr *not_xmm_e, struct Reg *not_xmm,
					   uc to_ss) {
	struct Reg *xmm = try_borrow_xmm_reg(not_xmm_e->tvar, g);

	ss_or_sd;
	reg_(xmm);
	reg_enter(not_xmm);

	free_register(not_xmm);
	return xmm;
}

struct Reg *xmm_bin_to_reg(Gg, struct LocalExpr *e, struct Reg *r1,
						   struct Reg *r2) {
	uc is_ss = is_ss(e->type);

	if (!is_xmm(r1))
		r1 = cvt_to_xmm(g, e->l, r1, is_ss);
	if (!is_xmm(r2))
		r2 = cvt_to_xmm(g, e->r, r2, is_ss);
	if (!is_ss) {
		if (is_ss(e->l->type))
			cvt_ss_to_sd(r1);
		if (is_ss(e->r->type))
			cvt_ss_to_sd(r2);
	}

	iprint_xmm_op(g, e->code, is_ss);
	reg_(r1);
	reg_enter(r2);

	free_reg(r2);
	return r1;
}

void to_ptr_arithmetic(struct LocalExpr **index, struct Pos *pos,
					   int ptr_targ_size) {
	struct Token *tok = new_tok(copy_blist_from_str("*"), MUL, pos);
	struct LocalExpr *ptr_arithmetic =
		new_local_expr(LE_BIN_MUL, copy_type_expr((*index)->type), tok);

	tok = new_tok(0, INT, pos);
	tok->num = ptr_targ_size, tok->view = int_to_str(ptr_targ_size);

	ptr_arithmetic->l = *index;
	ptr_arithmetic->r =
		new_local_expr(LE_PRIMARY_INT, new_type_expr(TC_I32), tok);

	opt_bin_constant_folding(ptr_arithmetic);
	*index = ptr_arithmetic;
}

void test_if_ptr_arithmetic(struct LocalExpr *e) {
	int ptr_targ_size;

	if (e->l->type->code == TC_PTR) {
		if (is_INT_le(e->r)) {
			e->r->tvar->num *= unsafe_size_of_type(ptr_targ(e->l->type));
			update_int_view(e->r);

		} else if (is_num_int_type(e->r->type)) {
			ptr_targ_size = unsafe_size_of_type(ptr_targ(e->l->type));
			to_ptr_arithmetic(&e->r, e->tvar->p, ptr_targ_size);
		}
	} else if (e->r->type->code == TC_PTR) {
		if (is_INT_le(e->l)) {
			e->l->tvar->num *= unsafe_size_of_type(ptr_targ(e->r->type));
			update_int_view(e->l);

		} else if (is_num_int_type(e->l->type)) {
			ptr_targ_size = unsafe_size_of_type(ptr_targ(e->r->type));
			to_ptr_arithmetic(&e->l, e->tvar->p, ptr_targ_size);
		}
	}
}

struct Reg *bin_to_reg(Gg, struct LocalExpr *e) {
	struct Reg *r1 = 0, *r2 = 0;
	struct LocalExpr *l, *r;
	struct LocalExpr *int_or_mem, *not_num;

	if (is_num_le(e->l) && is_num_le(e->r))
		exit(156);

	if (lceb(ADD) || lceb(SUB))
		test_if_ptr_arithmetic(e);

	l = e->l, r = e->r;

	if (is_num_le(l) && is_commut(e->code) ? (int_or_mem = l, not_num = r)
		: is_num_le(r)					   ? (int_or_mem = r, not_num = l)
		: lceb(SHR) || lceb(SHR)		   ? 0
		: is_mem(l) && is_commut(e->code)  ? (int_or_mem = l, not_num = r)
		: is_mem(r)						   ? (int_or_mem = r, not_num = l)
										   : 0) {
		r1 = gen_to_reg(g, not_num, 0);
		// is mem size is not equal to reg_size then can as well just get mem to
		// reg and do bin with other reg cuz may lose data if mem size is less
		int int_or_mem_size = unsafe_size_of_type(int_or_mem->type);

		if (!is_num_le(int_or_mem) && int_or_mem_size != r1->size) {
			r2 = gen_to_reg(g, int_or_mem, 0);
			goto two_regs;
		} else if (!is_xmm(r1))
			r1 = get_reg_to_size(g, r1, int_or_mem_size);

		// gen_mem_tuple can safely gen tuple for int or real too
		gen_mem_tuple(g, int_or_mem);

		if (is_real_type(e->type)) {
			if (lceep(int_or_mem, INT)) {
				int_or_mem->code = LE_PRIMARY_REAL;
				int_or_mem->tvar->real = int_or_mem->tvar->num;
				turn_type_to_simple(int_or_mem, e->type->code);
				r2 = prime_to_reg(g, int_or_mem, 0);
			} else if (lceep(int_or_mem, REAL))
				r2 = prime_to_reg(g, int_or_mem, 0);
			else
				r2 = gen_to_reg(g, int_or_mem, 0);
			return xmm_bin_to_reg(g, e, r1, r2);
		}
		if (lceb(DIV))
			return lceep(int_or_mem, INT) ? div_on_int(g, e, r1)
										  : div_on_mem(g, e, r1);
		if (lceb(MOD))
			return lceep(int_or_mem, INT) ? mod_on_int(g, e, r1)
										  : mod_on_mem(g, e, r1);
		if (lceb(MUL) && lceep(int_or_mem, INT))
			return mul_on_int(g, r1, int_or_mem->tvar->num);
		if ((lceb(SHR) || lceb(SHR)))
			return shift_on_int(g, e, r1);

		iprint_op(g, e->code);
		reg_(r1);
		if (lceb(MUL))
			reg_(r1);

		if (lceep(int_or_mem, INT)) {
			add_int_with_hex_comm(fun_text, int_or_mem->tvar->num);
		} else {
			mem_enter(int_or_mem, 0);
		}
		return r1;
	} else {
		if (!have_any_side_effect(l) && le_depth(l) < le_depth(r)) {
			r2 = gen_to_reg(g, r, 0);
			r1 = gen_to_reg(g, l, 0);
		} else {
			r1 = gen_to_reg(g, l, 0);
			r2 = gen_to_reg(g, r, 0);
		}
	two_regs:

		if (is_real_type(e->type))
			return xmm_bin_to_reg(g, e, r1, r2);
		if (lceb(SHR) || lceb(SHR))
			return shift_on_reg(g, e, r1, r2);

		get_regs_to_one_size(&r1, &r2);

		if (lceb(DIV))
			return div_on_reg(g, e, r1, r2);
		if (lceb(MOD))
			return mod_on_reg(g, e, r1, r2);

		iprint_op(g, e->code);
		reg_(r1);
		reg_enter(r2);

		free_register(r2);
		return r1;
	}
	exit(157);
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
