#include "pser.h"

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
