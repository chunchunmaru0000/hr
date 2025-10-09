#include "../pser.h"

struct LocalExpr *after_l_expression(struct Pser *p);
struct LocalExpr *prime_l_expression(struct Pser *p);
struct LocalExpr *unary_l_expression(struct Pser *p);
struct LocalExpr *mulng_l_expression(struct Pser *p);
struct LocalExpr *addng_l_expression(struct Pser *p);
struct LocalExpr *shtng_l_expression(struct Pser *p);
struct LocalExpr *mlsng_l_expression(struct Pser *p);
struct LocalExpr *equng_l_expression(struct Pser *p);
struct LocalExpr *b_and_l_expression(struct Pser *p);
struct LocalExpr *b_xor_l_expression(struct Pser *p);
struct LocalExpr *b_or__l_expression(struct Pser *p);
struct LocalExpr *l_and_l_expression(struct Pser *p);
struct LocalExpr *l_or__l_expression(struct Pser *p);
struct LocalExpr *trnry_l_expression(struct Pser *p) {
	absorb(p);
	return 0;
}
