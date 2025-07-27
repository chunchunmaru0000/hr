#include "pser.h"

long a = (long)(&"str");

struct GlobExpr *after_expression(struct Pser *p);
struct GlobExpr *prime_expression(struct Pser *p);
struct GlobExpr *unary_expression(struct Pser *p);
// struct GlobExpr *mulng_expression(struct Pser *p);
struct GlobExpr *addng_expression(struct Pser *p);
// struct GlobExpr *booly_expression(struct Pser *p);

#define global_expression(p) (after_expression((p)))
// "str" "str" "str" "str"
// num + num - num * num / num
// {struct values}
// окак [ч32] 123
struct GlobExpr *parse_global_expression(struct Pser *p, struct Arg *arg) {
	struct GlobExpr *e = global_expression(p);
	// enum Comp compatibility = get_types_compatibility(e->type, arg->type);

	// switch (compatibility) {
	// case C_COMPATIBLE:
	// 	break;
	// case C_SIZE_COMPATIBLE:
	// 	break;
	// case C_SIZE_UNCOMPATIBLE:
	// 	break;
	// case C_UNCOMPATIBLE:
	// 	eet(p->f, plist_get(arg->names, 0), "Несовместимые типы", "эээ");
	// 	break;
	// }

	return e;
}

void *expression(struct Pser *p) {
	// 	switch (t0->code) {
	// 	case INT:
	// 		set_tc(&t0, cp, t0, OINT, &o->sz, DWORD);
	// 		break;
	// 	case REAL:
	// 		set_tc(&t0, cp, t0, OFPN, &o->sz, QWORD);
	// 		break;
	// 	case STR:
	// 		if (t0->str->size == 1) {
	// 			t0->number = (uint64_t)t0->str->st[0];
	// 			o->sz = BYTE;
	// 		} else if (t0->str->size == 2) {
	// 			t0->number = *(uint16_t *)(t0->str->st);
	// 			o->sz = WORD;
	// 		} else
	// 			ee_token(p->f, t0, INVALID_STR_LEN);
	// 		break;
	// 	case MINUS:
	// 		t0 = next_get(p, -1);
	// 		if (t0->code == INT) {
	// 			t0->number *= -1;
	// 			code = OINT;
	// 		} else if (t0->code == REAL) {
	// 			t0->fpn *= -1;
	// 			code = OFPN;
	// 		} else
	// 			ee_token(p->f, t0, ERR_WRONG_MINUS);
	// 		o->sz = DWORD;
	// 		ot = t0;
	// 		break;
	// 	case ID:
	// 		break;
	// 	case PAR_L:
	// 		break;
	// 	default:
	// 		ee_token(p->f, t0, ERR_WRONG_TOKEN);
	// 	};
	//
	// 	return o;

	return p;
}

struct GlobExpr *after_expression(struct Pser *p) {
	struct GlobExpr *e = addng_expression(p);

	return e;
}
struct GlobExpr *prime_expression(struct Pser *p) {
	struct Token *c = pser_cur(p);

	struct GlobExpr *e = malloc(sizeof(struct GlobExpr));
	e->type = new_type_expr(TC_VOID);
	e->ops = new_plist(2);

	switch (c->code) {
	case INT:
	case ID:
	case REAL:
	case STR:
	case PAR_L:

	default:;
	}

	return e;
}
struct GlobExpr *unary_expression(struct Pser *p) {
	struct Token *c = pser_cur(p);
	struct GlobExpr *e = prime_expression(p);

	switch (c->code) {
	case AMPER:

	default:;
	}

	return e;
}

struct GlobExpr *addng_expression(struct Pser *p) {
	struct GlobExpr *e = unary_expression(p);

	return e;
}
