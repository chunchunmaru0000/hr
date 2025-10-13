#include "../pser.h"

struct LocalExpr *local_bin(struct Pser *p, struct LocalExpr *l,
							struct LocalExpr *r, struct Token *op) {
	return 0;
}

struct LocalExpr *unary_l_expression(struct Pser *p) {
	return prime_l_expression(p);
}

#define binop(prev_fun, cond)                                                  \
	do {                                                                       \
		struct LocalExpr *e = prev_fun(p);                                     \
		struct Token *c;                                                       \
                                                                               \
		loop {                                                                 \
			c = pser_cur(p);                                                   \
                                                                               \
			if (cond) {                                                        \
				consume(p);                                                    \
				e = local_bin(p, e, prev_fun(p), c);                           \
			} else                                                             \
				break;                                                         \
		}                                                                      \
                                                                               \
		return e;                                                              \
	} while (0)

// cce - C->Code Equal
#define cce(op) (c->code == (op))
#define ops1(o1) (cce(o1))
#define ops2(o1, o2) (cce(o1) || cce(o2))
#define ops3(o1, o2, o3) (cce(o1) || cce(o2) || cce(o3))
#define ops4(o1, o2, o3, o4) (cce(o1) || cce(o2) || cce(o3) || cce(o4))

// Binop Function
#define bf(name, next, ops)                                                    \
	struct LocalExpr *name(struct Pser *p) { binop(next, ops); }

bf(mulng_l_expression, unary_l_expression, ops3(MUL, DIV, MOD));
bf(addng_l_expression, mulng_l_expression, ops2(PLUS, MINUS));
bf(shtng_l_expression, addng_l_expression, ops2(SHL, SHR));
bf(mlsng_l_expression, shtng_l_expression, ops4(LESS, LESSE, MORE, MOREE));
bf(equng_l_expression, mlsng_l_expression, ops2(EQUE, NEQU));
bf(b_and_l_expression, equng_l_expression, ops1(AMPER));
bf(b_xor_l_expression, b_and_l_expression, ops1(BIT_XOR));
bf(b_or__l_expression, b_xor_l_expression, ops1(BIT_OR));
bf(l_and_l_expression, b_or__l_expression, ops1(AND));
bf(l_or__l_expression, l_and_l_expression, ops1(OR));

struct LocalExpr *trnry_l_expression(struct Pser *p) {
	return l_or__l_expression(p);
}
