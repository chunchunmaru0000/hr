#include "../../gner.h"
#include <stdio.h>

void cmp_bool(Gg, struct LocalExpr *e) {
	struct Reg *r1 = 0, *r2 = 0;
	if (lceb(SUB)) {
		just_cmp(g, e);
	} else if (lceb(ADD) && lceeu(e->r, MINUS)) {
		// 	// e - n -> e - n
		gen_tuple_of(g, e->r);
		e->r = e->r->l; // TODO: here may lose tuple
		just_cmp(g, e);
	} else if (lceb(ADD) && lceep(e->r, INT)) {
		// 	// e + n -> e - -n
		e->r->tvar->num = -e->r->tvar->num;
		just_cmp(g, e);
	} else {
		r1 = gen_to_reg(g, e, 0);
		op_reg_reg(TEST, r1, r1);
	}
	free_register(r1);
	free_register(r2);
}

//	LE_BIN_LESS 		setl 	setb
//	LE_BIN_LESSE 		setle 	setbe
//	LE_BIN_MORE 		setg 	seta
//	LE_BIN_MOREE 		setge 	setae
//	LE_BIN_EQUALS 		sete
//	LE_BIN_NOT_EQUALS 	setne
#define cbe(bin) (le == LE_BIN_##bin)
#define str_len_be(code) ((str = SA_##code, len = SA_##code##_LEN))

void iprint_set(Gg, enum LE_Code le, int is_u) {
	const char *str;
	u32 len;
	if (is_u) {
		cbe(LESS)		  ? str_len_be(SETB)
		: cbe(LESSE)	  ? str_len_be(SETBE)
		: cbe(MORE)		  ? str_len_be(SETA)
		: cbe(MOREE)	  ? str_len_be(SETAE)
		: cbe(EQUALS)	  ? str_len_be(SETE)
		: cbe(NOT_EQUALS) ? str_len_be(SETNE)
						  : exit(133);
	} else {
		cbe(LESS)		  ? str_len_be(SETL)
		: cbe(LESSE)	  ? str_len_be(SETLE)
		: cbe(MORE)		  ? str_len_be(SETG)
		: cbe(MOREE)	  ? str_len_be(SETGE)
		: cbe(EQUALS)	  ? str_len_be(SETE)
		: cbe(NOT_EQUALS) ? str_len_be(SETNE)
						  : exit(134);
	}
	indent_line(g, g->fun_text);
	blat(g->fun_text, (uc *)str, len - 1);
}

void just_cmp(Gg, struct LocalExpr *e) {
	struct Reg *r1 = 0, *r2 = 0;
	struct LocalExpr *l = e->l, *r = e->r;

	if (lceep(l, INT))
		exit(112); // shouldn't be, ozer flips l int to r

	else if (lceep(r, INT)) {
		if (is_mem(l)) {
			gen_mem_tuple(g, l);
			gen_tuple_of(g, r);
			op_mem_(CMP, l, 0);
		} else {
			r1 = gen_to_reg(g, l, 0);
			gen_tuple_of(g, r);
			op_reg_(CMP, r1->reg_code);
		}
		add_int_with_hex_comm(fun_text, r->tvar->num);

	} else if (is_mem(r)) {
		r1 = gen_to_reg(g, l, 0);
		gen_mem_tuple(g, r);

		op_reg_(CMP, r1->reg_code);
		mem_enter(r, 0);

	} else if (is_mem(l) && !have_any_side_effect(l)) {
		gen_tuple_of(g, l);
		r1 = gen_to_reg(g, r, 0);

		op_mem_(CMP, l, 0);
		reg_enter(r1->reg_code);

	} else {
		if (!have_any_side_effect(l) && le_depth(l) < le_depth(r)) {
			r2 = gen_to_reg(g, r, 0);
			r1 = gen_to_reg(g, l, 0);
		} else {
			r1 = gen_to_reg(g, l, 0);
			r2 = gen_to_reg(g, r, 0);
		}
		get_regs_to_one_size(&r1, &r2);
		op_reg_reg(CMP, r1, r2);
	}

	free_register(r2);
	free_register(r1);
}

struct Reg *xmm_cmp(Gg, struct LocalExpr *e);

struct Reg *cmp_with_set(Gg, struct LocalExpr *e) {
	struct Reg *r1 = 0;

	if (is_int_type(e->l->type) && is_int_type(e->r->type)) {
		just_cmp(g, e);

		iprint_set(g, e->code, is_u_type(e->type->code));
		r1 = try_borrow_reg(e->tvar, g, BYTE);
		reg_enter(r1->reg_code);
	} else {
	}
	if (!r1)
		exit(135);
	return r1;
}
