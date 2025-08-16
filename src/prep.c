#include "prep.h"

struct NodeToken *gen_node_tokens(struct PList *tokens) {
	struct NodeToken *head = malloc(sizeof(struct NodeToken));
	struct NodeToken *cur = head, *prev = 0;
	uint32_t i;

	head->prev = 0;
	for (i = 0; i < tokens->size; i++) {
		cur->token = plist_get(tokens, i);

		if (prev) {
			prev->next = cur;
			cur->prev = prev;
		}

		prev = cur;
	}
	cur->next = 0;

	return head;
}

void preprocess(struct Pser *p) {
	struct Prep *pr = malloc(sizeof(struct Prep));
	pr->pos = 0;
	pr->head = gen_node_tokens(p->ts);
	pr->macros = new_plist(10);
}
