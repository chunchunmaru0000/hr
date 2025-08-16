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
	struct PList *places;
	// place type PT_INSERT or PT_MERGE
};

struct Macro {
	struct Token *name;
	struct PList *args;
	struct PList *tokens;
};

struct Prep {
	uint32_t pos;
	struct NodeToken *head;

	struct PList *defines;
	struct PList *macros;
};

void preprocess(struct Pser *p);
