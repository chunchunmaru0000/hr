#include "pser.h"
#include "regs.h"
#include <stdint.h>

struct Register {
	const char *const name;
	uc len;
	enum RegCode reg_code;
	int size;
};

struct Gner {
	enum Target t;
	uc debug;

	uint32_t pos;
	struct PList *is;
	struct PList *global_vars;
	struct PList *local_vars;
	long stack_counter;

	struct BList *bprol; // before prolog
	struct BList *prol;	 // prolog
	struct BList *text;	 // main text

	struct BList *tmp_blist; // just tmp blist
};

struct Gner *new_gner(struct Pser *, enum Target, uc);
void gen(struct Gner *);

#define blat_str_prol(str) (blat(g->prol, (uc *)(str), (str##_LEN - 1)))
#define blat_str_bprol(str) (blat(g->bprol, (uc *)(str), (str##_LEN - 1)))
#define blat_str_text(str) (blat(g->text, (uc *)(str), (str##_LEN - 1)))
#define bprol_add(byte) (blist_add(g->bprol, (byte)))
#define prol_add(byte) (blist_add(g->prol, (byte)))
#define text_add(byte) (blist_add(g->text, (byte)))

#define num_add(list, value)                                                   \
	do {                                                                       \
		g->tmp_blist = num_to_str(value);                                      \
		blat_blist(list, g->tmp_blist);                                        \
		blist_clear_free(g->tmp_blist);                                        \
	} while (0);

void gen_Fasm_Linux_64_prolog(struct Gner *);
void gen_Fasm_Linux_64_text(struct Gner *);

void gen_Асм_Linux_64_prolog(struct Gner *);
void gen_Асм_Linux_64_text(struct Gner *);

struct LocalVar {
	struct Token *name;
	struct TypeExpr *type;
	long stack_pointer;
};

#define size_of_local(var) (size_of_type((var)->type))

struct LocalVar *new_local_var(struct Token *, struct TypeExpr *, long);
