#include "prep.h"
#include <stdio.h>

void pre(struct Prep *pr, struct PList *final_tokens);

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
	pr->f = p->f;
	pr->head = gen_node_tokens(p->ts);
	pr->defines = new_plist(10);
	pr->macros = new_plist(10);

	p->ts->size = 0;
	pre(pr, p->ts);
}

struct NodeToken *take_guaranteed_next(struct Prep *pr, struct NodeToken *n) {
	if (!n->next)
		eet(pr->f, n->token, WASNT_EXPECTING_EOF, 0);
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

void replace_token(struct Token *dst, struct Token *src) {
	dst->code = src->code;
	// pos stays same

	blist_clear_free(dst->view);
	dst->view = copy_str(src->view);
	if (dst->str)
		blist_clear_free(dst->str);
	dst->str = src->str ? copy_str(src->str) : 0;

	dst->num = src->num;
	dst->real = src->real;
}

#define foreach_begin(item, items)                                             \
	for (i = 0; i < items->size; i++) {                                        \
		item = plist_get(items, i);
#define foreach_end }

struct NodeToken *parse_vot(struct Prep *pr, struct NodeToken *c) {
	struct Define *define = malloc(sizeof(struct Define));

	// - parse statement
	c = take_guaranteed_next(pr, c);
	define->name = c->token; // TODO: should name be ID?
	c = take_guaranteed_next(pr, c);
	define->replace = c->token;

	// - save statement
	plist_add(pr->defines, define);

	return c;
}

const char *const STR_VOT = "вот";
const char *const STR_SE = "се";

struct NodeToken *try_parse_sh(struct Prep *pr, struct NodeToken *name) {
	if (name->token->code != ID)
		eet(pr->f, name->token, WAS_EXPECTING_PREP_INST_WORD, 0);

	if (vcs(name->token, STR_VOT))
		return parse_vot(pr, name);

	if (vcs(name->token, STR_SE)) {
		// - parse statement
		// - save statement
		// return parse_se(pr, name);
	}

	eet(pr->f, name->token, WAS_EXPECTING_PREP_INST_WORD, 0);
	return 0;
}

void apply_token(struct Prep *pr, struct NodeToken *c) {
	struct Define *define;
	struct Macro *macro;
	uint32_t i;

	foreach_begin(define, pr->defines);
	if (vc(define->name, c->token)) {
		replace_token(c->token, define->replace);
		goto replaced;
	}
	foreach_end;

	foreach_begin(macro, pr->macros);
	if (vc(macro->name, c->token)) {
		// call_macro(c, macro);
		goto replaced;
	}
	foreach_end;

replaced:;
}

void pre(struct Prep *pr, struct PList *final_tokens) {
	struct NodeToken *c, *name, *fst, *lst;

	for (c = pr->head; c;) {
		// printf("doin' %s\n", vs(c->token));
		if (c->token->code != SHARP) {
			apply_token(pr, c);
			c = c->next;
			continue;
		}

		fst = c;
		name = take_guaranteed_next(pr, c);
		lst = try_parse_sh(pr, name);

		// - cut off statemnt nodes so it wont be in final_tokens
		c = cut_off_inclusive(fst, lst); // its already next token in here
		if (pr->head == fst) // fst is freed but pointer is just a value
			pr->head = c;	 // in case if its first and tokens decapitated
	}

	// collect tokens
	for (c = pr->head; c; c = c->next)
		plist_add(final_tokens, c->token);
}
