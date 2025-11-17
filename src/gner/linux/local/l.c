#include "../../gner.h"
#include <stdio.h>

#define cbe(bin) (code == LE_BIN_##bin)
#define str_len_be(code) ((str = SA_##code, len = SA_##code##_LEN))
void iprint_op(Gg, enum LE_Code code) {
	const char *str;
	u32 len;
	cbe(MUL)	   ? str_len_be(MUL)
	: cbe(DIV)	   ? str_len_be(DIV)
	: cbe(ADD)	   ? str_len_be(ADD)
	: cbe(SUB)	   ? str_len_be(SUB)
	: cbe(SHL)	   ? str_len_be(SHL)
	: cbe(SHR)	   ? str_len_be(SHR)
	: cbe(BIT_AND) ? str_len_be(BIT_AND)
	: cbe(BIT_XOR) ? str_len_be(BIT_XOR)
	: cbe(BIT_OR)  ? str_len_be(BIT_OR)
				   : exit(155);
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
		reg = try_borrow_reg(e->tvar, g, reg_size);
		get_assignee_size(g, e, &gvar, &lvar);
		mov_reg_var(g, reg->reg_code, lvar, gvar);

	} else if (lcep(REAL)) {
		unit_size = e->type->code == TC_SINGLE ? DWORD : QWORD;

		reg = try_borrow_reg(e->tvar, g, unit_size);
		mov_reg_(g, reg->reg_code);
		real_add(g->fun_text, e->tvar->real);
		ft_add('\n');

		xmm = try_borrow_xmm_reg(e->tvar, g);
		mov_xmm_reg_(xmm->reg_code);
		sib(g, unit_size, 0, 0, reg->rf->r->reg_code, 0, 0), ft_add('\n');

		free_reg_family(reg->rf);
		reg = xmm;

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
		isprint_ft(NEG);
		reg_enter(reg->reg_code);
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

		mov_reg_(g, reg->reg_code);
		reg_enter(byte->reg_code);

		free_byte_reg(byte);
	} else if (lceu(BIT_NOT)) {
		reg = gen_to_reg(g, e->l, reg_size);
		isprint_ft(NOT);
		reg_enter(reg->reg_code);
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
/*
TODO:
(a + b):
	- mov reg, a
	- mov reg, b
	- add reg, reg
	+ mov reg, a
	+ add reg, b
opt mul to shift if possible
*/
struct Reg *bin_to_reg(Gg, struct LocalExpr *e, int reg_size) {
	struct Reg *r1 = 0, *r2 = 0;
	struct LocalExpr *l = e->l, *r = e->r;
	struct LocalExpr *num, *not_num;
	struct Reg *xmm, *not_xmm;

	if (is_num_le(l) && is_num_le(r))
		exit(156);

	if ((lceep(l, INT) ? (num = l, not_num = r) : 0) ||
		(lceep(r, INT) ? (num = r, not_num = l) : 0)) {
		gen_tuple_of(g, num);

		r1 = gen_to_reg(g, not_num, reg_size);
		iprint_op(g, e->code);
		reg_(r1->reg_code);
		if (lceb(MUL))
			reg_(r1->reg_code);

		add_int_with_hex_comm(fun_text, num->tvar->num);
		return r1;
	} else {
		r1 = gen_to_reg(g, l, reg_size);
		r2 = gen_to_reg(g, r, reg_size);

		if (is_xmm(r1) && is_xmm(r2))
			return xmm_bin_to_reg(g, r1, r2);
		if ((is_xmm(r1) ? (xmm = r1, not_xmm = r2) : 0) ||
			(is_xmm(r2) ? (xmm = r2, not_xmm = r1) : 0))
			return xmm_bin_to_reg(g, xmm, convert_to_xmm(not_xmm));

		iprint_op(g, e->code);
		reg_(r1->reg_code);
		if (lceb(MUL))
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
