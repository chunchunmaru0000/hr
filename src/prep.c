#include "prep.h"
#include <stdio.h>

void pre(struct Prep *pr, struct PList *final_tokens, struct Fpfc *f);
const char *const WASNT_EXPECTING_EOF = "Неожиданно встречен конец файла.";

struct NodeToken *gen_node_tokens(struct PList *tokens) {
	struct NodeToken *head = malloc(sizeof(struct NodeToken));
	struct NodeToken *cur = head, *prev = 0;
	uint32_t i;

	head->token = plist_get(tokens, 0);
	head->prev = 0;
	head->next = 0;
	prev = head;
	for (i = 1; i < tokens->size; i++) {
		cur = malloc(sizeof(struct NodeToken));
		cur->token = plist_get(tokens, i);
		cur->prev = prev;
		prev->next = cur;
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

	p->ts->size = 0;
	pre(pr, p->ts, p->f);
}

void pre(struct Prep *pr, struct PList *final_tokens, struct Fpfc *f) {
	struct NodeToken *c, *n;

	for (c = pr->head; c; c = c->next) {
		if (c->token->code != SHARP)
			continue; // TODO: try apply statement
		n = c->next;
		if (!c)
			eet(f, c->token, WASNT_EXPECTING_EOF, 0);

		// here need to:
		// - parse statement
		// - save statement
		// - cut off statemnt nodes so it wont be in final_tokens
	}

	// collect tokens
	for (c = pr->head; c; c = c->next)
		plist_add(final_tokens, c->token);
}
