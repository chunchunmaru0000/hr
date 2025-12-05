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
		mov_xmm_reg_(r->reg_code);
	} else {
		r = try_borrow_reg(e->tvar, g, reg_size);
		op_reg_(MOV, r->reg_code);
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
		mov_xmm_reg_(xmm->reg_code);

		free_reg_family(r->rf), r = xmm;
	} else {
		op_reg_(MOV, r->reg_code);
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
			op_reg_(MOV, reg->reg_code);
			real_add_enter(fun_text, e->tvar->real);
		} else {
			op_reg_reg(XOR, reg, reg);
		}

		xmm = try_borrow_xmm_reg(e->tvar, g);
		mov_xmm_reg_(xmm->reg_code);
		reg_enter(reg->reg_code);

		free_reg_family(reg->rf);
		reg = xmm;

	} else if (lcep(INT)) {
		reg = try_borrow_reg(e->tvar, g, reg_size);

		if (e->tvar->num) {
			op_reg_(MOV, reg->reg_code);
			add_int_with_hex_comm(fun_text, e->tvar->num);
		} else {
			op_reg_reg(XOR, reg, reg);
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
		op_reg_(LEA, r->reg_code);
		mem_enter(e, QWORD);
	} else if ((trailed = is_not_assignable_or_trailed(e))) {
		struct BList *last_mem_str = 0;
		lm_size = QWORD;
		r = gen_to_reg_with_last_mem(g, e, trailed, &last_mem_str);
		op_reg_(LEA, r->reg_code);
		last_mem_enter(last_mem_str);
		blist_clear_free(last_mem_str);
	}

	if (r == 0)
		exit(159);
	return r;
}

struct Reg *unary_to_reg(Gg, struct LocalExpr *e, int reg_size) {
	struct Reg *reg = 0, *byte;
	int unit_size;

	if (lceu(MINUS)) {
		reg = gen_to_reg(g, e->l, reg_size);
		op_reg_enter(NEG, reg->reg_code);
	} else if (lceu(INC) || lceu(DEC)) {
		reg = unary_dec_inc(g, e->l, lceu(INC));
	} else if (lceu(NOT) || lce(BOOL)) {
		byte = try_borrow_reg(e->tvar, g, BYTE);
		reg = cmp_with_int(g, e->l, 0);

		if (lce(BOOL))
			op_(SETNE);
		else
			op_(SETE);
		reg_enter(byte->reg_code);

		op_reg_reg(MOV, reg, byte);

		free_byte_reg(byte);
	} else if (lceu(BIT_NOT)) {
		reg = gen_to_reg(g, e->l, reg_size);
		op_reg_enter(NOT, reg->reg_code);
	} else if (lceu(AMPER)) {
		reg = dereference(g, e->l);
	} else if (lceu(ADDR)) {
		unit_size = unsafe_size_of_type(e->type);
		reg = gen_to_reg(g, e->l, 0); // QWORD by itself
		op_reg_(MOV, reg->reg_code);
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
	reg_(r->reg_code);
	reg_enter(xmm_reg->reg_code);
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
	reg_(xmm->reg_code);
	reg_enter(not_xmm->reg_code);

	free_reg_family(not_xmm->rf);
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
			cvt_ss_to_sd(r1->reg_code);
		if (is_ss(e->r->type))
			cvt_ss_to_sd(r2->reg_code);
	}

	iprint_xmm_op(g, e->code, is_ss);
	reg_(r1->reg_code);
	reg_enter(r2->reg_code);

	free_reg(r2);
	return r1;
}

#define get_regs_to_one_size(r1p, r2p)                                         \
	if ((*(r1p))->size > (*(r2p))->size)                                       \
		*(r2p) = get_reg_to_size(g, *(r2p), (*(r1p))->size);                   \
	else if ((*(r2p))->size > (*(r1p))->size)                                  \
		*(r1p) = get_reg_to_size(g, *(r1p), (*(r2p))->size);

struct Reg *bin_to_reg(Gg, struct LocalExpr *e) {
	struct Reg *r1 = 0, *r2 = 0;
	struct LocalExpr *l = e->l, *r = e->r;
	struct LocalExpr *int_or_mem, *not_num;

	if (is_num_le(l) && is_num_le(r))
		exit(156);

	if (is_num_le(l) && is_commut(e->code) ? (int_or_mem = l, not_num = r)
		: is_num_le(r)					   ? (int_or_mem = r, not_num = l)
		: lceb(SHR) || lceb(SHR)		   ? 0
		: is_mem(l) && is_commut(e->code)  ? (int_or_mem = l, not_num = r)
		: is_mem(r)						   ? (int_or_mem = r, not_num = l)
										   : 0) {
		r1 = gen_to_reg(g, not_num, 0);
		// is mem size is not equal to reg_size then can as well just get mem to
		// reg and do bin with other reg cuz may lose data if mem size is less
		if (!is_num_le(int_or_mem) &&
			unsafe_size_of_type(int_or_mem->type) != r1->size) {
			r2 = gen_to_reg(g, int_or_mem, 0);
			goto two_regs;
		}
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
		if (lceb(MUL) && lceep(int_or_mem, INT))
			return mul_on_int(g, r1, int_or_mem->tvar->num);
		if ((lceb(SHR) || lceb(SHR)))
			return shift_on_int(g, e, r1);

		iprint_op(g, e->code);
		reg_(r1->reg_code);
		if (lceb(MUL))
			reg_(r1->reg_code);

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

		if (lceb(DIV))
			return div_on_reg(g, e, r1, r2);
		if (lceb(SHR) || lceb(SHR))
			return shift_on_reg(g, e, r1, r2);

		get_regs_to_one_size(&r1, &r2);

		iprint_op(g, e->code);
		reg_(r1->reg_code);
		reg_enter(r2->reg_code);

		free_reg_family(r2->rf);
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
		res_reg = unary_to_reg(g, e, reg_size);
	else if (lcea(CALL))
		res_reg = call_to_reg(g, e, reg_size);
	else if (lcea(INC) || lcea(DEC))
		res_reg = after_dec_inc(g, e->l, lcea(INC));
	else if (lceb(AND))
		res_reg = and_to_reg(g, e, reg_size);
	else if (lceb(OR))
		res_reg = or_to_reg(g, e, reg_size);
	else if (is_uses_cmp(e))
		return cmp_with_set(g, e);
	else if (is_bin_le(e))
		res_reg = bin_to_reg(g, e);
	else if (lceb(TERRY))
		res_reg = terry_to_reg(g, e, reg_size);
	else if (lceb(ASSIGN))
		res_reg = assign_to_reg(g, e, reg_size);
	else
		exit(152);
	return is_xmm(res_reg) ? res_reg : get_reg_to_size(g, res_reg, reg_size);
}
