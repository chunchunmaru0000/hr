#include "../../gner.h"

struct Reg *cmp_with_int(Gg, struct LocalExpr *e, long num) {
	struct Reg *some_reg = 0;

	some_reg = gen_to_reg(g, e, 0);
	isprint_ft(CMP);
	reg_(some_reg->reg_code);
	add_int_with_hex_comm(fun_text, num);

	return some_reg;
}
