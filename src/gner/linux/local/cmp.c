#include "../../gner.h"

struct Reg *cmp_with_int(Gg, struct LocalExpr *e, long num) {
	struct Reg *some_reg = 0;

	some_reg = gen_to_reg(g, e, 0);
	isprint_ft(CMP);
	reg_(some_reg->reg_code);
	add_int_with_hex_comm(fun_text, num);

	return some_reg;
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
#define reverse_cmp_le(le)                                                     \
	((le) == LE_BIN_LESS	? LE_BIN_MORE                                      \
	 : (le) == LE_BIN_LESSE ? LE_BIN_MOREE                                     \
	 : (le) == LE_BIN_MORE	? LE_BIN_LESS                                      \
	 : (le) == LE_BIN_MOREE ? LE_BIN_LESSE                                     \
							: (le))

void just_cmp(Gg, struct LocalExpr *e) {
	struct Reg *r1 = 0, *r2 = 0;
	struct LocalExpr *l = e->l, *r = e->r;
	declare_lvar_gvar;

	if (lceep(l, INT))
		exit(112); // shouldn't be, ozer flips l int to r

	else if (lceep(r, INT)) {
		if (lceep(l, VAR)) {
			gen_tuple_of(g, l);
			gen_tuple_of(g, r);
			get_assignee_size(g, l, &gvar, &lvar);
			op_var_(CMP, lvar, gvar);
		} else {
			r1 = gen_to_reg(g, l, 0);
			gen_tuple_of(g, r);
			op_reg_(CMP, r1->reg_code);
		}
		add_int_with_hex_comm(fun_text, r->tvar->num);

	} else if (lceep(r, VAR)) {
		r1 = gen_to_reg(g, l, 0);
		gen_tuple_of(g, r);

		get_assignee_size(g, r, &gvar, &lvar);
		op_reg_(CMP, r1->reg_code);
		var_enter(lvar, gvar);

	} else if (lceep(l, VAR) && !have_any_side_effect(l)) {
		gen_tuple_of(g, l);
		r1 = gen_to_reg(g, r, 0);

		get_assignee_size(g, l, &gvar, &lvar);
		op_var_(CMP, lvar, gvar);
		reg_enter(r1->reg_code);

	} else {
		if (!have_any_side_effect(l) && le_depth(l) < le_depth(r)) {
			r2 = gen_to_reg(g, r, 0);
			r1 = gen_to_reg(g, l, 0);
		} else {
			r1 = gen_to_reg(g, l, 0);
			r2 = gen_to_reg(g, r, 0);
		}
		op_reg_reg(CMP, r1, r2);
	}

	if (r2)
		free_reg_family(r2->rf);
	if (r1)
		free_reg_family(r1->rf);
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
