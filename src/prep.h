#include "pser.h"

#define foreach_begin(item, items)                                             \
	for (i = 0; i < items->size; i++) {                                        \
		item = plist_get(items, i);
#define foreach_end }

struct NodeToken {
	struct Token *token; // value
	struct NodeToken *next;
	struct NodeToken *prev;
};

struct Define {
	struct Token *name;
	struct Token *replace;
};

struct MacroArg {
	struct Token *name;
	struct PList *usages;
	// place type PT_INSERT or PT_MERGE
};

struct Macro {
	struct Token *name;
	struct PList *args;

	struct NodeToken *fst;
	struct NodeToken *lst;
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
