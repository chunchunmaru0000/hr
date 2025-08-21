#include "pser.h"

#define foreach_begin(item, items)                                             \
	for (i = 0; i < items->size; i++) {                                        \
		item = plist_get(items, i);
#define foreach_end }

extern const char *const WASNT_EXPECTING_EOF;
extern const char *const WAS_EXPECTING_PREP_INST_WORD;
extern const char *const EXPCEPTED_PAR_L_OR_SH_L;
extern const char *const EXPECTED_ID_AS_MACRO_ARG;
extern const char *const EXPECTED_ID_AS_MACRO_NAME;

struct NodeToken {
	struct Token *token; // value
	struct NodeToken *next;
	struct NodeToken *prev;
};

struct Define {
	struct Token *name;
	struct Token *replace;
};

struct Nodes {
	struct NodeToken *fst;
	struct NodeToken *lst;
};
// for macro you first copy its tree and then for every usage of arg
// and then insert(replace inclusive) arg provided tokens in place of arg usage
struct MacroArg {
	struct Token *name;
	struct PList *usages; // its plist of NodeTokens
};

struct Macro {
	struct Token *name;
	struct PList *args;

	struct Nodes *body;
};

struct Prep {
	struct Fpfc *f;
	struct NodeToken *head;

	struct PList *defines;
	struct PList *macros;
};

extern struct PList *included_files; // list of BLists
struct PList *preprocess(struct Tzer *tzer);

struct NodeToken *gen_node_tokens(struct PList *tokens);

struct NodeToken *take_applyed_next(struct Prep *pr, struct NodeToken *c);
struct NodeToken *replace_inclusive(struct NodeToken *place,
									struct NodeToken *fst,
									struct NodeToken *lst);
void copy_nodes(struct Pos *place_pos, struct NodeToken *src_fst,
				struct NodeToken *src_lst, struct NodeToken **dst_fst,
				struct NodeToken **dst_lst);
void replace_token(struct Token *dst, struct Token *src);
struct NodeToken *cut_off_inclusive(struct NodeToken *fst,
									struct NodeToken *lst);
struct NodeToken *take_guaranteed_next(struct NodeToken *n);
struct NodeToken *clone_node_token(struct NodeToken *src);

struct NodeToken *parse_se(struct Prep *pr, struct NodeToken *c);
struct NodeToken *call_macro(struct Prep *pr, struct NodeToken *c,
							 struct Macro *macro);
