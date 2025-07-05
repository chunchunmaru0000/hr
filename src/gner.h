#include "pser.h"
#include <stdint.h>

struct Gner {
	enum Target t;
	uc debug;

	struct PList *is;
	uint32_t pos;

	struct BList *bprol;
	struct BList *prol;
	struct BList *text;
};

struct Gner *new_gner(struct PList *, enum Target, uc);
void gen(struct Gner *);

#define blat_str_prol(g, str, str_len)                                         \
	(blat((g)->prol, (uc *)(str), (str_len) - 1))
#define blat_str_bprol(g, str, str_len)                                        \
	(blat((g)->bprol, (uc *)(str), (str_len) - 1))
#define bprol_add(g, byte) (blist_add(g->bprol, byte))
#define prol_add(g, byte) (blist_add(g->prol, byte))

void gen_Fasm_Linux_64_prolog(struct Gner *);
void gen_Fasm_Linux_64_text(struct Gner *);

void gen_Асм_Linux_64_prolog(struct Gner *);
void gen_Асм_Linux_64_text(struct Gner *);

struct BList *num_to_str(long num);
