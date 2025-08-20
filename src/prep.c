#include "prep.h"
#include <stdio.h>

#define foreach_begin(item, items)                                             \
	for (i = 0; i < items->size; i++) {                                        \
		item = plist_get(items, i);
#define foreach_end }

void pre(struct Prep *pr, struct PList *final_tokens);
struct NodeToken *take_applyed_next(struct Prep *pr, struct NodeToken *c);
struct NodeToken *gen_node_tokens(struct PList *tokens);
void replace_token(struct Token *dst, struct Token *src);
const char *const STR_INCLUDE = "влечь";

const char *const WASNT_EXPECTING_EOF = "Неожиданно встречен конец файла.";
const char *const WAS_EXPECTING_PREP_INST_WORD =
	"После токена '#' ожидалось одно из ключевых слов препроцессора: "
	"'вот', 'влечь' или 'се'.";
const char *const EXPCEPTED_PAR_L_OR_SH_L =
	"В объявлении макро ожидалось либо '(' для начала аргументов либо '(#' для "
	"начало текста макро.";
const char *const EXPECTED_STR_AS_AN_INCLUDE_PATH =
	"После инструкции препроцессора '#влечь' ожидалась строка содержащая путь "
	"файла.";
const char *const EXPECTED_ID_AS_MACRO_ARG =
	"Ожидалось слово в качестве аргумента для макро.";
const char *const EXPECTED_ID_AS_MACRO_NAME =
	"Ожидалось слово в качестве имени для макро.";

#define ALREADY_INCLUDED -1

struct BList *try_include_path(struct Token *path) {
	struct BList *included_path;
	uint32_t i;

	foreach_begin(included_path, included_files);
	// here compare token str cuz view has "
	if (sc(ss(path), (char *)included_path->st))
		return 0;
	foreach_end;

	included_path = path->str;
	plist_add(included_files, included_path);

	return included_path;
}

struct NodeToken *get_included_head(struct BList *included_path) {
	struct NodeToken *included_head;
	struct PList *new_tokens;
	struct Tzer *tzer;

	tzer = new_tzer((char *)included_path->st);
	new_tokens = tze(tzer, 128);
	// printf("# gen_node_tokens in %s with %d num of tokens\n",
	// 	   (char *)included_path->st, new_tokens->size);
	included_head = gen_node_tokens(new_tokens);

	free(tzer);
	plist_free(new_tokens); // here every EF token is lost
	return included_head;
}

struct NodeToken *try_include(struct NodeToken *prev, struct Token *path) {
	struct NodeToken *included_head;

	struct BList *path_to_include = try_include_path(path);
	if (!path_to_include)
		return (void *)ALREADY_INCLUDED;

	included_head = get_included_head(path_to_include);
	// printf("## got included_head with token %s, prev now is %p\n",
	// 	   vs(included_head->token), prev);
	if (included_head->token->code == EF)
		return prev;

	included_head->prev = prev;
	if (prev)
		prev->next = included_head;

	// here need to return tail, cuz head already attached to the prev
	// so it will proceed gen loop from the tail
	// also need to skip EOF token in here
	// printf("## in %s with head of %s and next %s\n", vs(path),
	// 	   vs(included_head->token), vs(included_head->next->token));
	while (included_head->next && included_head->token->code != EF)
		included_head = included_head->next;
	// printf("### after a while with head of %s and next %p\n",
	// 	   vs(included_head->token), included_head->next);

	if (included_head->token->code == EF) {
		// printf("#### in %s returns head of %s\n", vs(path),
		// 	   vs(included_head->prev->token));
		// TODO: free new_head->new_head and its token
		return included_head->prev;
	} else
		exit(19);
}

struct NodeToken *try_parse_include(struct NodeToken *prev,
									struct PList *tokens, uint32_t i) {
	struct Token *token;

	token = plist_get(tokens, i);
	if (token->code != SHARP || i + 2 >= tokens->size)
		return 0;

	token = plist_get(tokens, ++i);
	if (token->code != ID || !vcs(token, STR_INCLUDE))
		return 0;

	token = plist_get(tokens, ++i);
	if (token->code != STR)
		eet(token, EXPECTED_STR_AS_AN_INCLUDE_PATH, 0);

	return try_include(prev, token);
}

struct NodeToken *gen_node_tokens(struct PList *tokens) {
	struct NodeToken *head; // = malloc(sizeof(struct NodeToken));
	struct NodeToken *cur = head, *prev = 0;
	uint32_t i;

	for (i = 0; i < tokens->size; i++) {
		cur = try_parse_include(prev, tokens, i);

		if (cur == 0)
			goto just_token;
		if (cur == (void *)ALREADY_INCLUDED)
			goto skip_include;
		if (cur) {
			// printf("##### included with cur of %s\n", vs(cur->token));
			if (prev == 0)
				head = cur;
			else if (prev != cur)
				prev->next = cur;
			// printf("##### head now is %s\n", vs(head->token));
			prev = cur;
		skip_include:
			i += 2; // skip 'влечь' and string path literal
			continue;
		}

	just_token:
		cur = malloc(sizeof(struct NodeToken));
		cur->token = plist_get(tokens, i);
		cur->prev = prev;
		if (prev == 0)
			head = cur;
		else
			prev->next = cur;
		prev = cur;
	}
	cur->next = 0;

	// printf("###### returns from gen_node_tokens with head of %s\n",
	// 	   vs(head->token));
	return head;
}

struct PList *included_files = 0;

struct PList *preprocess(struct Tzer *tzer) {
	struct Prep *pr = malloc(sizeof(struct Prep));
	pr->f = tzer->f;
	pr->defines = new_plist(10);
	pr->macros = new_plist(10);
	if (included_files == 0)
		included_files =
			new_plist(8); // TODO: in here need to have source file path too

	struct PList *tokens = tze(tzer, 128);
	free(tzer);

	pr->head = gen_node_tokens(tokens);
	tokens->size = 0;
	pre(pr, tokens);

	return tokens;
}

struct NodeToken *take_guaranteed_next(struct NodeToken *n) {
	if (!n->next)
		eet(n->token, WASNT_EXPECTING_EOF, 0);
	return n->next;
}

void free_node_token(struct NodeToken *n) {
	// TODO: actually maybe i need to free some tokens but dunno
	// anyway no need for this now
	// but at least its known memory leak
	free(n);
}

struct NodeToken *clone_node_token(struct NodeToken *src) {
	struct NodeToken *dst = malloc(sizeof(struct NodeToken));
	memcpy(dst, src, sizeof(struct NodeToken));
	return dst;
}

struct Token *deep_clone_token(struct Token *src, struct Pos *pos) {
	struct Token *dst = malloc(sizeof(struct Token));

	dst->view = 0;
	dst->str = 0;
	replace_token(dst, src);
	dst->p = pos;

	return dst;
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
	if (dst->view)
		blist_clear_free(dst->view);
	dst->view = copy_str(src->view);

	if (dst->str)
		blist_clear_free(dst->str);
	dst->str = src->str ? copy_str(src->str) : 0;

	dst->num = src->num;
	dst->real = src->real;
}

void copy_nodes(struct Pos *place_pos, struct NodeToken *src_fst,
				struct NodeToken *src_lst, struct NodeToken **dst_fst,
				struct NodeToken **dst_lst) {
	struct NodeToken *copy_head, *prev_copy, *fst_copy;

	// here need to copy token so it will have pos of a place
	copy_head = clone_node_token(src_fst);
	copy_head->token = deep_clone_token(src_fst->token, place_pos);
	prev_copy = copy_head;

	for (src_fst = src_fst->next; src_fst != src_lst; src_fst = src_fst->next) {
		fst_copy = clone_node_token(src_fst);
		fst_copy->token = deep_clone_token(src_fst->token, place_pos);
		fst_copy->prev = prev_copy;
		prev_copy->next = fst_copy;

		prev_copy = fst_copy;
	}
	fst_copy = clone_node_token(src_lst);
	fst_copy->token = deep_clone_token(src_lst->token, place_pos);
	fst_copy->prev = prev_copy;
	prev_copy->next = fst_copy;

	*dst_fst = copy_head;
	*dst_lst = fst_copy;
}

struct NodeToken *replace_inclusive(struct NodeToken *place,
									struct NodeToken *fst,
									struct NodeToken *lst) {
	if (fst == lst) {
		replace_token(place->token, fst->token);
		return place;
	}

	struct NodeToken *fst_copy, *lst_copy;
	copy_nodes(place->token->p, fst, lst, &fst_copy, &lst_copy);

	place->prev->next = fst_copy;
	fst_copy->prev = place->prev;

	place->next->prev = lst_copy;
	lst_copy->next = place->next;

	free_node_token(place);
	return fst_copy;
}

struct NodeToken *parse_vot(struct Prep *pr, struct NodeToken *c) {
	struct Define *define = malloc(sizeof(struct Define));

	// - parse statement
	c = take_guaranteed_next(c);
	define->name = c->token; // TODO: should name be ID?
	c = take_guaranteed_next(c);
	define->replace = c->token;

	// - save statement
	plist_add(pr->defines, define);

	return c;
}

struct NodeToken *parse_macro_args(struct Prep *pr, struct NodeToken *c,
								   struct Macro *macro) {
	struct MacroArg *arg;

	macro->args = new_plist(4);
	c = take_guaranteed_next(c); // skip '('

	for (; c->token->code != PAR_R; c = take_guaranteed_next(c)) {
		if (c->token->code != ID)
			eet(c->token, EXPECTED_ID_AS_MACRO_ARG, 0);

		arg = malloc(sizeof(struct MacroArg));
		arg->name = c->token;
		arg->usages = new_plist(2);

		plist_add(macro->args, arg);
	}
	// here c should be ')'
	return take_guaranteed_next(c);
}

struct NodeToken *parse_macro_block(struct Prep *pr, struct NodeToken *c,
									struct Macro *macro) {
	struct NodeToken *clone, *clone_prev;

	c = take_guaranteed_next(c); // skip '(#'

	// here need to copy tree cuz it will be freed from final tokens
	clone = clone_node_token(c); // first is different
	clone->prev = 0;
	macro->fst = clone;

	clone_prev = clone;
	c = take_applyed_next(pr, c);
	for (; c->token->code != SH_R; c = take_applyed_next(pr, c)) {
		clone = clone_node_token(c);
		clone->prev = clone_prev;
		clone_prev->next = clone;

		clone_prev = clone;
	}
	clone->next = 0;
	macro->lst = clone;

	return c; // need to return '#)' cuz it will be last
}

struct NodeToken *parse_se(struct Prep *pr, struct NodeToken *c) {
	struct Macro *macro = malloc(sizeof(struct Macro));

	// - parse statement
	c = take_guaranteed_next(c);
	if (c->token->code != ID)
		eet(c->token, EXPECTED_ID_AS_MACRO_NAME, 0);
	macro->name = c->token;

	c = take_guaranteed_next(c);
	// parse args
	if (c->token->code == SH_L)
		macro->args = 0;
	else if (c->token->code == PAR_L) {
		c = parse_macro_args(pr, c, macro);
		expect(c->token, SH_L);
	} else
		eet(c->token, EXPCEPTED_PAR_L_OR_SH_L, 0);
	// parse block
	c = parse_macro_block(pr, c, macro);

	// - save statement
	plist_add(pr->macros, macro);

	return c;
}

const char *const STR_VOT = "вот";
const char *const STR_SE = "се";

// TODO: if redefine then free last one
struct NodeToken *try_parse_sh(struct Prep *pr, struct NodeToken *name) {
	if (name->token->code != ID)
		eet(name->token, WAS_EXPECTING_PREP_INST_WORD, 0);

	if (vcs(name->token, STR_VOT))
		return parse_vot(pr, name);

	if (vcs(name->token, STR_SE))
		return parse_se(pr, name);

	eet(name->token, WAS_EXPECTING_PREP_INST_WORD, 0);
	return 0;
}

struct NodeToken *take_applyed_next(struct Prep *pr, struct NodeToken *c) {
	struct Define *define;
	struct Macro *macro;
	uint32_t i;

	foreach_begin(define, pr->defines);
	if (vc(define->name, c->token)) {
		replace_token(c->token, define->replace);
		return take_applyed_next(pr, c);
	}
	foreach_end;

	foreach_begin(macro, pr->macros);
	if (vc(macro->name, c->token)) {
		if (macro->args) {
			exit(200); // call_macro(pr, c, macro);
		} else {
			c = replace_inclusive(c, macro->fst, macro->lst);
		}
		return take_applyed_next(pr, c);
	}
	foreach_end;

	return c->next;
}

void pre(struct Prep *pr, struct PList *final_tokens) {
	struct NodeToken *c, *name, *fst, *lst;

	for (c = pr->head; c;) {
		// printf("doin' %s\n", vs(c->token));
		if (c->token->code != SHARP) {
			c = take_applyed_next(pr, c);
			continue;
		}

		fst = c;
		name = take_guaranteed_next(c);
		lst = try_parse_sh(pr, name);

		// - cut off statemnt nodes so it wont be in final_tokens
		c = cut_off_inclusive(fst, lst); // its already next token in here
		if (pr->head == fst) // fst is freed but pointer is just a value
			pr->head = c;	 // in case if its first and tokens decapitated
	}

	// collect tokens
	for (c = pr->head; c; c = c->next)
		plist_add(final_tokens,
				  c->token); // TODO: check if its still have EF token
}
