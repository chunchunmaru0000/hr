#include "ozer.h"

/*
MUL, DIV [ MOD, WHOLE_DIV ]
PLUS, MINUS
[ SHL, SHR ]
[ LESS, LESSE, MORE, MOREE ]
[ EQUE, NEQU ]
[ AMPER ]
[ BIT_XOR ]
[ BIT_OR ]
[ AND ]
[ OR ]
[ TERRY ]
[ EQU ]
[ PIPE_LINE ]
*/

void opt_bin_constant_folding(struct LocalExpr *e) {}

struct PList *opt_local_expr(struct LocalExpr *e) {
	opt_bin_constant_folding(e);
	return 0;
}
