#include "pser.h"
#include <stdint.h>

struct Gner {
	enum Target t;
	uc debug;

	uint32_t pos;
	struct PList *is;
	struct PList *global_vars;

	struct BList *bprol;
	struct BList *prol;
	struct BList *text;
};

struct Gner *new_gner(struct Pser *, enum Target, uc);
void gen(struct Gner *);

#define blat_str_prol(g, str) (blat((g)->prol, (uc *)(str), (str##_LEN - 1)))
#define blat_str_bprol(g, str) (blat((g)->bprol, (uc *)(str), (str##_LEN - 1)))
#define blat_str_text(g, str) (blat((g)->text, (uc *)(str), (str##_LEN - 1)))
#define bprol_add(g, byte) (blist_add(g->bprol, byte))
#define prol_add(g, byte) (blist_add(g->prol, byte))

void gen_Fasm_Linux_64_prolog(struct Gner *);
void gen_Fasm_Linux_64_text(struct Gner *);

void gen_Асм_Linux_64_prolog(struct Gner *);
void gen_Асм_Linux_64_text(struct Gner *);

struct BList *num_to_str(long num);
