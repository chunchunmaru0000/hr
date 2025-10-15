#include "../pser.h"
#include <stdio.h>

struct LocalExpr *new_local_expr(enum LE_Code le_code, struct TypeExpr *type,
								 struct Token *tvar, uint32_t ops_size) {
	struct LocalExpr *e = malloc(sizeof(struct LocalExpr));
	e->code = le_code;
	e->type = type;
	e->tvar = tvar;
	e->ops = new_plist(ops_size);
	return e;
}

struct LEtoT {
	enum LE_Code le;
	enum TCode t;
};
#define leto(t)                                                                \
	{ LE_BIN_##t, t }

const struct LEtoT lets[] = {
	leto(MUL),
	leto(DIV),
	leto(MOD),
	leto(PLUS),
	leto(MINUS),
	leto(SHL),
	leto(SHR),
	leto(LESS),
	leto(LESSE),
	leto(MORE),
	leto(MOREE),
	{LE_BIN_EQUALS, EQUE},
	{LE_BIN_NOT_EQUALS, NEQU},
	{LE_BIN_BIT_AND, AMPER},
	leto(BIT_XOR),
	leto(BIT_OR),
	leto(AND),
	leto(OR),
	//{LE_BIN_TERRY, QUEST},
	{LE_BIN_ASSIGN, EQU},
};

struct LocalExpr *local_bin(struct Pser *p, struct LocalExpr *l,
							struct LocalExpr *r, struct Token *op) {
	const struct LEtoT *let;
	uint32_t i;
	enum TCode op_code = op->code;

	struct LocalExpr *e = new_local_expr(LE_NONE, 0, op, 2);

	plist_add(e->ops, l);
	plist_add(e->ops, r);

	for (i = 0, let = lets; i < loa(lets); i++, let++) {
		if (let->t == op_code) {
			e->code = let->le;
			return e;
		}
	}

	eet(op, "че за op", 0);
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
	struct LocalExpr * true, *false;
	struct LocalExpr *e = l_or__l_expression(p), *trnry;
	struct Token *c;

	c = pser_cur(p);

	if (c->code == QUEST) {
		consume(p);

		true = local_expression(p);
		match(pser_cur(p), COLO);
		false = local_expression(p);

		trnry = new_local_expr(LE_BIN_TERRY, 0, c, 3);
		plist_add(trnry->ops, e);
		plist_add(trnry->ops, true);
		plist_add(trnry->ops, false);

		e = trnry;
	}

	return e;
}
bf(assng_l_expression, trnry_l_expression, ops1(EQU));
