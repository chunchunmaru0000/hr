#include "prep.h"
#include <stdio.h>

void pre(struct Prep *pr, struct PList *final_tokens, struct Fpfc *f);

const char *const WASNT_EXPECTING_EOF = "Неожиданно встречен конец файла.";
const char *const WAS_EXPECTING_PREP_INST_WORD =
	"После токена '#' ожидалось одно из ключевых слов препроцессора: "
	"'вот' или 'се'.";

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
	pr->defines = new_plist(10);
	pr->macros = new_plist(10);

	p->ts->size = 0;
	pre(pr, p->ts, p->f);
}

struct NodeToken *take_guaranteed_next(struct Fpfc *f, struct NodeToken *n) {
	if (!n->next)
		eet(f, n->token, WASNT_EXPECTING_EOF, 0);
	return n->next;
}

void free_node_token(struct NodeToken *n) {
	// TODO: actually maybe i need to free some tokens but dunno
	// anyway no need for this now
	// but at least its known memory leak
	free(n);
}

// lst is guaranteed
struct NodeToken *cut_off_inclusive(struct NodeToken *fst,
									struct NodeToken *lst) {
	struct NodeToken *res, *tmp;
	if (fst->prev) {
		res = fst->prev;
		res->next = lst->next;
		res->next->prev = res;
	} else {
		res = lst->next;
		res->prev = 0;
	}

	for (tmp = fst; tmp != lst; tmp = fst) {
		fst = tmp->next;
		// printf("about to free: %s\n", vs(tmp->token));
		free_node_token(tmp);
	}
	// here tmp == lst
	// printf("about to free lst: %s\n", vs(tmp->token));
	free_node_token(tmp); // so frees lst

	return res;
}

const char *const STR_VOT = "вот";
const char *const STR_SE = "се";

void pre(struct Prep *pr, struct PList *final_tokens, struct Fpfc *f) {
	struct NodeToken *c, *fst, *lst;
	struct Token *t;

	struct Define *define;
	struct Macro *macro;

	for (c = pr->head; c;) {
		//	printf("doin': %s\n", vs(c->token));

		if (c->token->code != SHARP) {
			// TODO: try apply statement
			// so here need to search for defines and macros
			// and apply them
			c = c->next;
			continue;
		}

		fst = c;
		c = take_guaranteed_next(f, c);

		t = c->token;
		if (t->code != ID)
			eet(f, t, WAS_EXPECTING_PREP_INST_WORD, 0);

		// here need to:
		if (vcs(t, STR_VOT)) {
			define = malloc(sizeof(struct Define));

			// - parse statement
			c = take_guaranteed_next(f, c);
			define->name = c->token;
			c = take_guaranteed_next(f, c);
			define->replace = c->token;

			// - save statement
			plist_add(pr->defines, define);

			lst = c;
		} else if (vcs(t, STR_SE)) {
			// - parse statement
			// - save statement
		} else
			eet(f, t, WAS_EXPECTING_PREP_INST_WORD, 0);

		// - cut off statemnt nodes so it wont be in final_tokens
		c = cut_off_inclusive(fst, lst); // its already next token in here
		if (pr->head == fst) // fst is freed but pointer is just a value
			pr->head = c;	 // in case if its first and tokens decapitated
	}

	// collect tokens
	for (c = pr->head; c; c = c->next)
		plist_add(final_tokens, c->token);
}
