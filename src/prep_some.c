#include "prep.h"
#include <stdio.h>

// #################################################################
//					BELOW IS PREP STRINGIFY
// #################################################################

struct NodeToken *sh_string(struct NodeToken *c) {
	struct NodeToken *fst = c, *snd = take_guaranteed_next(c), *lst;
	struct Token *str_token = malloc(sizeof(struct Token));

	str_token->view = new_blist(32);
	str_token->code = STR;
	str_token->p = fst->token->p;

	str_token->num = 0;
	str_token->real = 0;
	str_token->str = new_blist(32);

	blist_add(str_token->view, '"');
	c = snd; // snd here already first take_guaranteed_next(c)
	while (c->token->code != SH_QR) {
		blat_blist(str_token->view, c->token->view);
		blat_blist(str_token->str, c->token->view);
		blist_add(str_token->view, ' ');
		blist_add(str_token->str, ' ');

		c = take_guaranteed_next(c);
	}
	lst = c;

	blist_add(str_token->view, '"');
	convert_blist_to_blist_from_str(str_token->view);
	convert_blist_to_blist_from_str(str_token->str);

	cut_off_inclusive(snd, lst);
	replace_token(fst->token, str_token);

	return fst;
}

// #################################################################
//					BELOW IS PREP TOKENS ADD
// #################################################################

const char *const TOO_MUCH_TOKENS =
	"В результате сложения токенов может быть только один токен.";
const char *const WAS_TOKENS = "токенов было ";

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
	struct PList *tokens;
	struct Token *res;
	// create tzer
	struct Tzer *tzer = malloc(sizeof(struct Tzer));
	tzer->f = plus->p->f;
	tzer->p = plus->p;
	tzer->pos = 0;
	// create tzer code
	t1->view->size++; // it should be zero terminated so
	blat_blist(t0->view, t1->view);
	tzer->code = vs(t0);
	tzer->clen = t0->view->size;
	// tze and free
	tokens = tze(tzer, 8);
	free(tzer);

	long tokens_size_without_ef = tokens->size - 1;
	if (tokens_size_without_ef != 1) {
		struct ErrorInfo ei = {t0, TOO_MUCH_TOKENS, WAS_TOKENS,
							   (void *)tokens_size_without_ef, ET_INT};
		etei_with_extra(&ei);
		exit(1);
		// eet(t0, TOO_MUCH_TOKENS, 0);
	}

	res = plist_get(tokens, 0);
	full_free_token(plist_get(tokens, 1)); // free EF token
	plist_free(tokens);

	return res;
}

struct NodeToken *shplus(struct Prep *pr, struct NodeToken *plus) {
	struct NodeToken *next = next_applyed(pr, plus);
	struct NodeToken *prev = plus->prev; // its applyed already

	if (!prev) // just dont add, im too lazy to do err here
		return next;

	struct Token *new_token = add_tokens(plus->token, prev->token, next->token);
	cut_off_inclusive(plus, next);

	new_token->p = prev->token->p;
	replace_token(prev->token, new_token);

	return prev;
}
