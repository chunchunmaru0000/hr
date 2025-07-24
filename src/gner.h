#include "pser.h"
#include "regs.h"
#include <stdint.h>

struct Register {
	const char *const name;
	uc len;
	enum RegCode reg_code;
	int size;
};

struct Fggs {
	uc is_stack_used;
	uc is_rbx_used;
	uc is_r12_used;
	// arguments only in registers, can be only done if there is no other args
	// in the function and no stack usage
	uc is_args_in_regs;
};

struct Gner {
	enum Target t;
	uc debug;

	uint32_t pos;
	long stack_counter;
	struct Fggs *flags;

	struct PList *is;
	struct PList *defines; // Defn's
	struct PList *structs; // Inst's of IP_DECLARE_STRUC
	struct PList *global_vars;
	struct PList *local_vars;

	struct GlobVar *current_function;
	struct PList *local_labels; // plist of tokens with labels names

	struct BList *bprol; // before prolog
	struct BList *prol;	 // prolog
	struct BList *text;	 // main text

	struct BList *tmp_blist; // just tmp blist
};

struct Gner *new_gner(struct Pser *, enum Target, uc);
void gen(struct Gner *);

// SA - Str Asm
extern const char SA_SEGMENT[];
extern const char SA_LABEL_END[];
extern const uint32_t SA_SEGMENT_LEN;
extern const uint32_t SA_LABEL_END_LEN;

extern const char SA_EQU[];
extern const char SA_ENTER_STACK_FRAME[];
extern const char SA_MOV_MEM_RBP_OPEN[];
extern const char SA_START_COMMENT[];
extern const char SA_SUB_RSP[];
extern const char SA_LEAVE[];
extern const char SA_RET[];
extern const char SA_JMP[];

extern const uint32_t SA_EQU_LEN;
extern const uint32_t SA_ENTER_STACK_FRAME_LEN;
extern const uint32_t SA_LEAVE_STACK_FRAME_LEN;
extern const uint32_t SA_MOV_MEM_RBP_OPEN_LEN;
extern const uint32_t SA_START_COMMENT_LEN;
extern const uint32_t SA_SUB_RSP_LEN;
extern const uint32_t SA_LEAVE_LEN;
extern const uint32_t SA_RET_LEN;
extern const uint32_t SA_JMP_LEN;

#define blat_str_bprol(str) (blat(g->bprol, (uc *)(str), (str##_LEN - 1)))
#define blat_str_prol(str) (blat(g->prol, (uc *)(str), (str##_LEN - 1)))
#define blat_str_text(str) (blat(g->text, (uc *)(str), (str##_LEN - 1)))
#define bprol_add(byte) (blist_add(g->bprol, (byte)))
#define prol_add(byte) (blist_add(g->prol, (byte)))
#define text_add(byte) (blist_add(g->text, (byte)))

#define num_add(list, value)                                                   \
	do {                                                                       \
		g->tmp_blist = num_to_str((value));                                    \
		blat_blist((list), g->tmp_blist);                                      \
		blist_clear_free(g->tmp_blist);                                        \
	} while (0);
#define num_hex_add(list, value)                                               \
	do {                                                                       \
		g->tmp_blist = num_to_hex_str((value));                                \
		blat_blist((list), g->tmp_blist);                                      \
		blist_clear_free(g->tmp_blist);                                        \
	} while (0);

void gen_Асм_Linux_64_prolog(struct Gner *);
void gen_Асм_Linux_64_text(struct Gner *);

void gen_local_Асм_Linux_64(struct Gner *g, struct Inst *in);

struct LocalVar {
	struct Token *name;
	struct TypeExpr *type;
	long stack_pointer;
};

#define size_of_local(var) (size_of_type((var)->type))

struct LocalVar *new_local_var(struct Token *, struct TypeExpr *, long);
void free_and_clear_local_vars(struct Gner *g);
