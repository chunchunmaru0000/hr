#include "../../gner.h"

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
	struct Reg *reg = 0;
	declare_lvar_gvar;

	if (lcep(VAR)) {
		reg = try_borrow_reg(e->tvar, g->cpu, reg_size);
		get_assignee_size(g, e, &gvar, &lvar);
		mov_reg_var(g, reg->reg_code, lvar, gvar);
	} else
		exit(145);

	return reg;
}

struct Reg *bin_to_reg(Gg, struct LocalExpr *e, int reg_size) {
	struct Reg *r1 = 0, *r2 = 0;
	struct LocalExpr *l = e->l, *r = e->r;
	struct LocalExpr *num, *not_num;

	if (is_num_le(l) && is_num_le(r))
		exit(156);

	if ((is_num_le(l) ? (num = l, not_num = r) : 0) ||
		(is_num_le(r) ? (num = r, not_num = l) : 0)) {

		r1 = gen_to_reg(g, not_num, reg_size);
		iprint_op(g, e->code);
		reg_(r1->reg_code);
		if (lceb(MUL))
			reg_(r1->reg_code);

		if (lceep(num, REAL))
			num->tvar->num = num->tvar->real;
		add_int_with_hex_comm(fun_text, num->tvar->num);

		return r1;
	} else {
		r1 = gen_to_reg(g, l, reg_size);
		r2 = gen_to_reg(g, r, reg_size);

		iprint_op(g, e->code);
		reg_(r1->reg_code);
		if (lceb(MUL))
			reg_(r1->reg_code);
		reg_(r2->reg_code), g->fun_text->size--, ft_add('\n');

		free_reg_family(r2->rf);
		return r1;
	}
	exit(157);
}

struct Reg *gen_to_reg(Gg, struct LocalExpr *e, uc of_size) {
	int reg_size = of_size ? of_size : unsafe_size_of_type(e->type);
	struct Reg *res_reg;

	if (is_primary(e))
		res_reg = prime_to_reg(g, e, reg_size);
	else if (is_bin_le(e))
		res_reg = bin_to_reg(g, e, reg_size);

	return res_reg;
}
