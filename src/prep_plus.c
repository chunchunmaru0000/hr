#include "prep.h"
#include <stdio.h>

// 	if (t0->code == STR && t1->code == STR) {
// 		t0->view->size--;
// 		t1->view->st++;
// 		t1->view->size -= 2;
// 		blat_blist(t0->view, t1->view);
// 		blat_blist(t0->str, t1->str);
//
// 		blist_clear_free(t1->view);
// 		blist_clear_free(t1->str);
// 		free(t1->p);
// 		return t0;
// 	}
struct Token *add_tokens(struct Token *plus, struct Token *t0,
						 struct Token *t1) {
	struct Tzer *t = malloc(sizeof(struct Tzer));
	t->f = plus->p->f;
	t->p = plus->p;
	t->pos = 0;

	t1->view->size++; // it should be zero terminated so
	blat_blist(t0->view, t1->view);

	t->code = vs(t0);

	full_free_token(t0);
	full_free_token(plus);
	full_free_token(t1);
}

struct NodeToken *shplus(struct Prep *pr, struct NodeToken *c) {
	struct NodeToken *next = next_applyed(pr, c);
	struct NodeToken *prev = c->prev; // its applyed already

	if (!prev) // just dont add, im too lazy to do err here
		return next;

	struct Token *new_token = add_tokens(c->token, prev->token, next->token);
	replace_token(prev->token, new_token);

	c = cut_off_inclusive(c, next);
	return prev;
}
