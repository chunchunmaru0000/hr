#include "pser.h"

#define foreach_begin(item, items)                                             \
	for (i = 0; i < items->size; i++) {                                        \
		item = plist_get(items, i);
#define foreach_by(count, item, items)                                         \
	for ((count) = 0; (count) < items->size; (count)++) {                      \
		item = plist_get(items, (count));
#define foreach_end }

extern const char *const WASNT_EXPECTING_EOF;
extern const char *const WAS_EXPECTING_PREP_INST_WORD;
extern const char *const EXPCEPTED_PAR_L_OR_SH_L;
extern const char *const EXPECTED_ID_AS_MACRO_ARG;
extern const char *const EXPECTED_ID_AS_MACRO_NAME;
extern const char *const CANT_HAVE_EMPTY_ARG_YET;

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

struct Macro {
	struct Token *name;
	struct PList *args;
	struct Nodes *body;
};

struct SentenceWord {
	struct Token *word;
	struct SentenceWord *or_word;
};
struct SentenceArg {
	struct Token *token;
	uint32_t index;
};

struct Sentence {
	struct PList *args;	 // plist of SentenceArg's
	struct PList *words; // plist of SentenceWord's
	struct Nodes *body;
};

struct Prep {
	struct Fpfc *f;
	struct NodeToken *head;

	struct PList *defines;
	struct PList *macros;
	struct PList *sentences;
};

struct NodeToken *gen_node_tokens(struct PList *tokens);
extern struct PList *included_files; // list of BLists
extern struct NodeToken *new_included_head;
struct PList *preprocess(struct Tzer *tzer);

struct NodeToken *parse_se(struct Prep *pr, struct NodeToken *c);
struct NodeToken *parse_include(struct NodeToken *c);
struct NodeToken *parse_sent(struct Prep *pr, struct NodeToken *name);
struct NodeToken *call_macro(struct NodeToken *c, struct Macro *macro);
struct NodeToken *shplus(struct Prep *pr, struct NodeToken *c);
struct NodeToken *sh_string(struct NodeToken *c);
struct NodeToken *call_macro(struct NodeToken *c, struct Macro *macro);
int try_apply_sentence(struct Prep *pr, struct NodeToken **c);

struct NodeToken *take_guaranteed_next(struct NodeToken *n);
struct NodeToken *next_of_line(struct NodeToken *e, struct NodeToken *n);
struct NodeToken *try_apply(struct Prep *pr, struct NodeToken *c);
#define next_applyed(pr, c) (try_apply((pr), take_guaranteed_next((c))))
struct Nodes *parse_body(struct NodeToken **start);
struct Nodes *gen_body(struct Nodes *body, struct PList *args,
					   struct PList *args_nodes);

struct NodeToken *replace_inclusive(struct NodeToken *place,
									struct NodeToken *fst,
									struct NodeToken *lst);
#define replace_nodes_inclusive(place, nodes)                                  \
	(replace_inclusive((place), (nodes)->fst, (nodes)->lst))
void copy_nodes(struct Pos *place_pos, struct NodeToken *src_fst,
				struct NodeToken *src_lst, struct NodeToken **dst_fst,
				struct NodeToken **dst_lst);
struct Nodes *copy_nodeses(struct Pos *place_pos, struct Nodes *src);
void replace_token(struct Token *dst, struct Token *src);
struct NodeToken *cut_off_inclusive(struct NodeToken *fst,
									struct NodeToken *lst);
struct NodeToken *clone_node_token(struct NodeToken *src);
struct NodeToken *deep_clone_node_with_pos(struct NodeToken *src,
										   struct Pos *pos);
struct NodeToken *deep_clone_node(struct NodeToken *src);
struct Token *deep_clone_token(struct Token *src, struct Pos *pos);
void full_free_node_token(struct NodeToken *n);
struct Nodes *new_nodes(struct NodeToken *fst, struct NodeToken *lst);
