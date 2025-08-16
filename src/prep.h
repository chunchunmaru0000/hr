#include "pser.h"

struct NodeToken {
	struct Token *token; // value
	struct NodeToken *next;
	struct NodeToken *prev;
};

struct Macro {};

struct Prep {
	uint32_t pos;

	struct NodeToken *head;
	struct PList *macros;
};

void preprocess(struct Pser *p);
