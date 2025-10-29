#include "../prep/prep.h"

struct PList *opt_local_expr(struct LocalExpr *e);

int try_opt_mul(struct LocalExpr *e);
int try_opt_div(struct LocalExpr *e);
int try_opt_add_or_sub(struct LocalExpr *e);
int try_opt_and(struct LocalExpr *e);
int try_opt_or(struct LocalExpr *e);
int try_opt_bit_or(struct LocalExpr *e);
int try_opt_bit_and(struct LocalExpr *e);

#define if_opted(cap, low) ((e->code == LE_BIN_##cap && try_opt_##low(e)))
#define if_opted2(cap0, cap1, low)                                             \
	(((e->code == LE_BIN_##cap0 || e->code == LE_BIN_##cap1) &&                \
	  try_opt_##low(e)))
