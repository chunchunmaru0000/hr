#include "regs.h"

#define uc unsigned char

struct Reg {
	struct BList *name;
	enum RegCode reg_code;
	uc size;
	uc allocated;
};

struct RegisterFamily {
	struct Reg *l; // 8 bit Lower
	struct Reg *h; // 8 bit High
	struct Reg *x; // 16 bit
	struct Reg *e; // 32 bit Extended
	struct Reg *r; // 64 bit
};

struct CPU {
	struct RegisterFamily *a;
	struct RegisterFamily *c;
	struct RegisterFamily *d;
	struct RegisterFamily *b;
	struct RegisterFamily *sp;
	struct RegisterFamily *bp;
	struct RegisterFamily *si;
	struct RegisterFamily *di;
	struct RegisterFamily *rex[8];
	struct Reg *xmm[16];
};
