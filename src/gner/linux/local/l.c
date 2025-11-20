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

// local var, arr(not ptr), field(not ptr)
// global var, arr(not ptr), field(not ptr)

struct Reg *prime_to_reg(Gg, struct LocalExpr *e, int reg_size) {
	struct Reg *reg = 0, *xmm;
	int unit_size;
	declare_lvar_gvar;

	if (lcep(VAR)) {
		get_assignee_size(g, e, &gvar, &lvar);

		if (is_real_type(e->type)) {
			reg = try_borrow_xmm_reg(e->tvar, g);
			mov_xmm_reg_(reg->reg_code);
			var_enter(lvar, gvar);
		} else {
			reg = try_borrow_reg(e->tvar, g, reg_size);
			mov_reg_var(g, reg->reg_code, lvar, gvar);
		}
	} else if (lcep(REAL)) {
		unit_size = e->type->code == TC_SINGLE ? DWORD : QWORD;

		reg = try_borrow_reg(e->tvar, g, unit_size);
		mov_reg_(g, reg->reg_code);
		real_add(g->fun_text, e->tvar->real);
		ft_add('\n');

		xmm = try_borrow_xmm_reg(e->tvar, g);
		mov_xmm_reg_(xmm->reg_code);
		reg_enter(reg->reg_code);

		free_reg_family(reg->rf);
		reg = xmm;

	} else if (lcep(INT)) {
		reg = try_borrow_reg(e->tvar, g, reg_size);
		mov_reg_(g, reg->reg_code);
		add_int_with_hex_comm(fun_text, e->tvar->num);

	} else
		exit(145);
	return reg;
}

struct Reg *dereference(Gg, struct LocalExpr *e) {
	struct Reg *reg = 0;
	declare_lvar_gvar;

	// local var, arr(not ptr), field(not ptr)
	// global var, arr(not ptr), field(not ptr)

	if (lcep(VAR)) {
		get_assignee_size(g, e, &gvar, &lvar);
		reg = try_borrow_reg(e->tvar, g, QWORD);

		if (lvar) {
			isprint_ft(LEA);
			reg_(reg->reg_code);
			sib(g, QWORD, R_RBP, 0, 0, (long)lvar->name->view, 1);
		} else {
			mov_reg_(g, reg->reg_code);
			blat_ft(gvar->signature);
		}
		ft_add('\n');
	}

	if (reg == 0)
		exit(159);
	return reg;
}

struct Reg *unary_to_reg(Gg, struct LocalExpr *e, int reg_size) {
	struct Reg *reg = 0, *byte;
	int unit_size;

	if (lceu(MINUS)) {
		reg = gen_to_reg(g, e->l, reg_size);
		op_reg_enter(NEG, reg->reg_code);
	} else if (lceu(INC)) {
	} else if (lceu(DEC)) {
	} else if (lceu(NOT) || lce(BOOL)) {
		byte = try_borrow_reg(e->tvar, g, BYTE);
		reg = cmp_with_int(g, e->l, 0);

		if (lce(BOOL))
			isprint_ft(SETNE);
		else
			isprint_ft(SETE);
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
		mov_reg_(g, reg->reg_code);
		sib(g, unit_size, 0, 0, reg->reg_code, 0, 0), ft_add('\n');
	}

	if (reg == 0)
		exit(158);
	return reg;
}

#define ss_or_sd                                                               \
	if (to_ss)                                                                 \
		isprint_ft(CVTSI2SS);                                                  \
	else                                                                       \
		isprint_ft(CVTSI2SD);

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

struct Reg *bin_to_reg(Gg, struct LocalExpr *e, int reg_size) {
	struct Reg *r1 = 0, *r2 = 0;
	struct LocalExpr *l = e->l, *r = e->r;
	struct LocalExpr *num, *not_num;

	if (is_num_le(l) && is_num_le(r))
		exit(156);

	if (lceep(l, INT) && is_commut(e->code) ? (num = l, not_num = r)
		: lceep(r, INT)						? (num = r, not_num = l)
											: 0) {
	int_or_var:
		gen_tuple_of(g, num);
		r1 = gen_to_reg(g, not_num, reg_size);

		if (is_real_type(e->type)) {
			if (lceep(num, INT)) {
				num->code = LE_PRIMARY_REAL;
				num->tvar->real = num->tvar->num;
				turn_type_to_simple(num, e->type->code);
				r2 = prime_to_reg(g, num, 0);
			} else
				r2 = prime_to_reg(g, num, reg_size);
			return xmm_bin_to_reg(g, e, r1, r2);
		}
		if (lceb(DIV))
			return lceep(num, INT) ? div_on_int(g, e, r1)
								   : div_on_mem(g, e, r1);
		if (lceb(MUL) && lceep(num, INT))
			return mul_on_int(g, r1, num);
		if ((lceb(SHR) || lceb(SHR)))
			return shift_on_int(g, e, r1);

		iprint_op(g, e->code);
		reg_(r1->reg_code);
		if (lceb(MUL))
			reg_(r1->reg_code);

		if (lceep(num, VAR)) {
			declare_lvar_gvar;
			get_assignee_size(g, num, &gvar, &lvar);
			var_enter(lvar, gvar);
		} else {
			add_int_with_hex_comm(fun_text, num->tvar->num);
		}
		return r1;
	} else {
		if (lceb(SHR) || lceb(SHR)				  ? 0
			: lceep(l, VAR) && is_commut(e->code) ? (num = l, not_num = r)
			: lceep(r, VAR)						  ? (num = r, not_num = l)
												  : 0)
			goto int_or_var;

		if (!have_any_side_effect(l) && le_depth(l) < le_depth(r)) {
			r2 = gen_to_reg(g, r, reg_size);
			r1 = gen_to_reg(g, l, reg_size);
		} else {
			r1 = gen_to_reg(g, l, reg_size);
			r2 = gen_to_reg(g, r, reg_size);
		}

		if (is_real_type(e->type))
			return xmm_bin_to_reg(g, e, r1, r2);

		if (lceb(DIV))
			return div_on_reg(g, e, r1, r2);
		if (lceb(SHR) || lceb(SHR))
			return shift_on_reg(g, e, r1, r2);

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
	struct Reg *res_reg;

	// TODO: use gen_tuple_of whenever is need
	gen_tuple_of(g, e);

	if (is_primary(e))
		res_reg = prime_to_reg(g, e, reg_size);
	else if (is_unary(e) || lce(BOOL))
		res_reg = unary_to_reg(g, e, reg_size);
	else if (is_bin_le(e))
		res_reg = bin_to_reg(g, e, reg_size);
	else
		exit(152);

	return res_reg;
}
