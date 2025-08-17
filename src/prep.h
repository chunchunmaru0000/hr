#include "pser.h"

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

void preprocess(struct Pser *p);
