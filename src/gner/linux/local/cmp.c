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
struct Reg *reverse_cmp_on_mem_or_int(Gg, struct LocalExpr *e, struct Reg *r1,
									  struct LocalExpr *num_or_mem) {
	enum LE_Code le = e->code;
	e->code = lceb(LESS)	? LE_BIN_MORE
			  : lceb(LESSE) ? LE_BIN_MOREE
			  : lceb(MORE)	? LE_BIN_LESS
			  : lceb(MOREE) ? LE_BIN_LESSE
							: e->code;
	r1 = cmp_on_mem_or_int(g, e, r1, num_or_mem);
	e->code = le; // in any case
	return r1;
}

struct Reg *cmp_on_mem_or_int(Gg, struct LocalExpr *e, struct Reg *r1,
							  struct LocalExpr *num_or_mem) {
	if (lceep(num_or_mem, INT)) {
		cmp_with_num(r1, num_or_mem);
	} else {
		declare_lvar_gvar;
		cmp_with_mem(r1, num_or_mem);
	}

	iprint_set(g, e->code, is_u_type(e->type->code));
	if (r1->rf->l) {
		r1 = r1->rf->l;
	} else {
		free_reg_family(r1->rf);
		r1 = try_borrow_reg(e->tvar, g, BYTE);
	}
	reg_enter(r1->reg_code);

	return r1;
}

struct Reg *cmp_on_reg(Gg, struct LocalExpr *e, struct Reg *r1,
					   struct Reg *r2) {
	cmp_with_reg(r1, r2);

	iprint_set(g, e->code, is_u_type(e->type->code));
	if (r1->rf->l) {
		free_reg_family(r2->rf);
		r1 = r1->rf->l;
	} else if (r2->rf->l) {
		free_reg_family(r1->rf);
		r1 = r2->rf->l;
	} else {
		free_reg_family(r1->rf);
		free_reg_family(r2->rf);
		r1 = try_borrow_reg(e->tvar, g, BYTE);
	}
	reg_enter(r1->reg_code);

	return r1;
}
