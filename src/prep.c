#include "prep.h"
#include <stdio.h>

void pre(struct Prep *pr, struct PList *final_tokens);
void replace_token(struct Token *dst, struct Token *src);

const char *const WASNT_EXPECTING_EOF = "Неожиданно встречен конец файла.";
const char *const WAS_EXPECTING_PREP_INST_WORD =
	"После токена '#' ожидалось одно из ключевых слов препроцессора: "
	"'вот', 'влечь' или 'се'.";
const char *const EXPCEPTED_PAR_L_OR_SH_L =
	"В объявлении макро ожидалось либо '(' для начала аргументов либо '(#' для "
	"начало текста макро.";
const char *const EXPECTED_ID_AS_MACRO_ARG =
	"Ожидалось слово в качестве аргумента для макро.";
const char *const EXPECTED_ID_AS_MACRO_NAME =
	"Ожидалось слово в качестве имени для макро.";

void add_source_path(struct Fpfc *f) {
	uint32_t path_len = strlen(f->path);
	char *path_copy = malloc(path_len + 1);
	memcpy(path_copy, f->path, path_len + 1);

	struct BList *source_path = blist_from_str(path_copy, path_len + 1);
	source_path->size--;
	plist_add(included_files, source_path);
}
struct PList *included_files = 0;

struct PList *preprocess(struct Tzer *tzer) {
	struct Prep *pr = malloc(sizeof(struct Prep));
	pr->f = tzer->f;
	pr->defines = new_plist(10);
	pr->macros = new_plist(10);

	if (included_files == 0) {
		included_files = new_plist(8);
		add_source_path(pr->f);
	}

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
// #define copy_nodeses(place_pos, str, dst)

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
#define replace_nodes_inclusive(place, nodes)                                  \
	(replace_inclusive((place), (nodes)->fst, (nodes)->lst))

struct NodeToken *try_apply(struct Prep *pr, struct NodeToken *c) {
	struct Define *define;
	struct Macro *macro;
	uint32_t i;

	foreach_begin(define, pr->defines);
	if (vc(define->name, c->token)) {
		replace_token(c->token, define->replace);
		return try_apply(pr, c);
	}
	foreach_end;

	foreach_begin(macro, pr->macros);
	if (vc(macro->name, c->token)) {
		if (macro->args) {
			c = call_macro(pr, c, macro);
		} else {
			c = replace_nodes_inclusive(c, macro->body);
		}
		// like its important cuz if new head also is kinda macro so
		return try_apply(pr, c);
	}
	foreach_end;

	return c;
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

void pre(struct Prep *pr, struct PList *final_tokens) {
	struct NodeToken *c, *name, *fst, *lst;

	for (c = pr->head; c;) {
		// printf("doin' %s\n", vs(c->token));
		if (c->token->code != SHARP) {
			c = try_apply(pr, c)->next;
			continue;
		}

		fst = c;
		// TODO: do i want here next_applyed or just take_guaranteed_next
		// like you now by this it can even redefine macro defenition
		name = take_guaranteed_next(c);
		lst = try_parse_sh(pr, name);

		// - cut off statemnt nodes so it wont be in final_tokens
		c = cut_off_inclusive(fst, lst); // its already next token in here
		if (pr->head == fst) // fst is freed but pointer is just a value
			pr->head = c;	 // in case if its first and tokens decapitated
		// try_apply(pr, c); // TODO: dunno, prev todo is about same thing
	}

	// collect tokens
	for (c = pr->head; c; c = c->next)
		plist_add(final_tokens, c->token);
}
