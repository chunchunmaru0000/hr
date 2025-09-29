#include "prep.h"
#include <stdio.h>

void pre(struct Prep *pr, struct PList *final_tokens);
void free_prep(struct Prep *pr);
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
const char *const SH_QR_CANT_OCCUR_BY_ITSELF =
	"Токен '\"#' не может быть использован отдельно, только в качестве "
	"закрываюшего токена для '#\"', например: #\"текст текст 123 текст\"#.";

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
	pr->sentences = new_plist(10);

	if (included_files == 0) {
		included_files = new_plist(8);
		add_source_path(pr->f);
	}

	struct PList *tokens = tze(tzer, 128);
	free(tzer);

	pr->head = gen_node_tokens(tokens);
	tokens->size = 0;
	pre(pr, tokens);

	free_prep(pr);
	return tokens;
}

void free_node_token(struct NodeToken *n);
void free_sentence_word(struct SentenceWord *w);
void free_prep(struct Prep *pr) {
	struct Macro *macro;
	struct Sentence *sent;
	struct SentenceWord *word;
	struct SentenceArg *sent_arg;
	struct Token *macro_arg;
	struct Define *define;
	struct NodeToken *node, *tmp_node;
	uint32_t i, j;
	// ################ DEFINES ##################
	foreach_begin(define, pr->defines);
	free(define);
	foreach_end;

	plist_free(pr->defines);
	// ################ MACROS ##################
	foreach_by(i, macro, pr->macros);
	full_free_token_without_pos(macro->name);
	if (macro->args) {
		foreach_by(j, macro_arg, macro->args);
		full_free_token_without_pos(macro_arg);
		foreach_end;
		plist_free(macro->args);
	}
	for (node = macro->body->fst; node; node = tmp_node) {
		tmp_node = node->next;
		free_node_token(node);
	}
	free(macro->body);
	free(macro);
	foreach_end;

	plist_free(pr->macros);
	// ################ SENTENCES ##################
	foreach_by(i, sent, pr->sentences);
	foreach_by(j, word, sent->words);
	if (word)
		free_sentence_word(word);
	foreach_end;
	foreach_by(j, sent_arg, sent->args);
	full_free_token_without_pos(sent_arg->token);
	free(sent_arg);
	foreach_end;
	for (node = sent->body->fst; node; node = tmp_node) {
		tmp_node = node->next;
		free_node_token(node);
	}
	plist_free(sent->words);
	plist_free(sent->args);
	free(sent->body);
	free(sent);
	foreach_end;

	plist_free(pr->sentences);
	free(pr);
}

struct Nodes *new_nodes(struct NodeToken *fst, struct NodeToken *lst) {
	struct Nodes *nodes = malloc(sizeof(struct Nodes));
	nodes->fst = fst;
	nodes->lst = lst;
	return nodes;
}

void free_sentence_word(struct SentenceWord *w) {
	struct SentenceWord *or_word = w->or_word;
	full_free_token_without_pos(w->word);
	if (or_word)
		free_sentence_word(or_word);
}

struct NodeToken *take_guaranteed_next(struct NodeToken *n) {
	if (!n->next)
		eet(n->token, WASNT_EXPECTING_EOF, 0);
	return n->next;
}
// not use next_of_line and replace it with nol_with_err
struct NodeToken *next_of_line(struct NodeToken *e, struct NodeToken *n) {
	if (!n->next)
		eet(e->token, WASNT_EXPECTING_EOF, 0);
	return n->next;
}
// Next Of Line
struct NodeToken *nol_with_err(struct NodeToken *e, struct NodeToken *n,
							   const char *const err) {
	if (!n->next)
		eet(e->token, err, 0);
	return n->next;
}

void full_free_node_token(struct NodeToken *n) {
	full_free_token(n->token);
	free(n);
}

void free_node_token(struct NodeToken *n) {
	full_free_token_without_pos(n->token);
	free(n);
}
// void free_node_token_new(struct NodeToken *n) {
// 	printf("# INFO. free_node_token_new |%s|\n", vs(n->token));
// 	free(n);
// }

struct NodeToken *clone_node_token(struct NodeToken *src) {
	struct NodeToken *dst = malloc(sizeof(struct NodeToken));
	memcpy(dst, src, sizeof(struct NodeToken));
	dst->token = deep_clone_token(src->token, src->token->p);
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

struct NodeToken *deep_clone_node_with_pos(struct NodeToken *src,
										   struct Pos *pos) {
	struct NodeToken *dst = clone_node_token(src);
	dst->token = deep_clone_token(src->token, pos);
	return dst;
}
struct NodeToken *deep_clone_node(struct NodeToken *src) {
	struct NodeToken *dst = clone_node_token(src);
	dst->token = deep_clone_token(src->token, src->token->p);
	return dst;
}

int both_zeros(struct NodeToken *fst, struct NodeToken *lst) {
	if (fst == 0 || lst == 0) {
		if (fst == 0 && lst == 0) {
			return 1;
		} else {
			printf("why one of nodes is zero\n");
			exit(177);
		}
	}
	return 0;
}

// WHEN USE NO NEED TO MALLOC, it allocs for you, need only ptr to set value in
// it cuz returns two values
void copy_nodes(struct Pos *place_pos, struct NodeToken *src_fst,
				struct NodeToken *src_lst, struct NodeToken **dst_fst,
				struct NodeToken **dst_lst) {
	struct NodeToken *copy_head, *prev_copy, *fst_copy;

	if (both_zeros(src_fst, src_lst)) {
		*dst_fst = 0;
		*dst_lst = 0;
		return;
	}

	// here need to copy token so it will have pos of a place
	copy_head = place_pos ? deep_clone_node_with_pos(src_fst, place_pos)
						  : deep_clone_node(src_fst);
	prev_copy = copy_head;

	if (src_fst == src_lst) {
		*dst_fst = copy_head;
		*dst_lst = copy_head;
		return;
	}

	for (src_fst = src_fst->next; src_fst != src_lst; src_fst = src_fst->next) {
		fst_copy = place_pos ? deep_clone_node_with_pos(src_fst, place_pos)
							 : deep_clone_node(src_fst);
		fst_copy->prev = prev_copy;
		prev_copy->next = fst_copy;

		prev_copy = fst_copy;
	}
	fst_copy = place_pos ? deep_clone_node_with_pos(src_lst, place_pos)
						 : deep_clone_node(src_lst);
	fst_copy->prev = prev_copy;
	prev_copy->next = fst_copy;

	*dst_fst = copy_head;
	*dst_lst = fst_copy;
}
struct Nodes *copy_nodeses(struct Pos *place_pos, struct Nodes *src) {
	struct Nodes *dst = new_nodes(0, 0);
	copy_nodes(place_pos, src->fst, src->lst, &dst->fst, &dst->lst);
	return dst;
}

struct NodeToken *replace_inclusive(struct NodeToken *place,
									struct NodeToken *fst,
									struct NodeToken *lst) {
	struct NodeToken *fst_copy, *lst_copy;

	if (both_zeros(fst, lst)) {
		if (place->prev) {
			fst_copy = place->prev;
			fst_copy->next = place->next;
			if (fst_copy->next)
				fst_copy->next->prev = fst_copy;
		} else {
			if (place->next == 0)
				exit(178);
			fst_copy = place->next;
		}
		free_node_token(place);
		return fst_copy;
	}

	if (fst == lst) {
		replace_token(place->token, fst->token);
		return place;
	}

	copy_nodes(place->token->p, fst, lst, &fst_copy, &lst_copy);

	if (place->prev)
		place->prev->next = fst_copy;
	fst_copy->prev = place->prev;

	place->next->prev = lst_copy;
	lst_copy->next = place->next;

	free_node_token(place);
	return fst_copy;
}

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
			c = call_macro(c, macro);
		} else {
			c = replace_nodes_inclusive(c, macro->body);
		}
		// like its important cuz if new head also is kinda macro so
		return try_apply(pr, c);
	}
	foreach_end;

	if (try_apply_sentence(pr, &c))
		return try_apply(pr, c);

	return c;
}

struct NodeToken *parse_vot(struct Prep *pr, struct NodeToken *c) {
	struct Define *define = malloc(sizeof(struct Define));

	// - parse statement
	c = take_guaranteed_next(c);
	define->name = deep_clone_token(c->token, c->token->p);
	c = take_guaranteed_next(c);
	define->replace = deep_clone_token(c->token, c->token->p);

	// - save statement
	plist_add(pr->defines, define);

	return c;
}

const char *const STR_VOT = "вот";
const char *const STR_SE = "се";
const char *const STR_INCLUDE = "влечь";
const char *const STR_SENTENCE = "буки";

// TODO: if redefine then free last one
struct NodeToken *try_parse_sh(struct Prep *pr, struct NodeToken *name) {
	if (name->token->code != ID)
		eet(name->token, WAS_EXPECTING_PREP_INST_WORD, 0);

	if (vcs(name->token, STR_VOT))
		return parse_vot(pr, name);

	if (vcs(name->token, STR_SE))
		return parse_se(pr, name);

	if (vcs(name->token, STR_INCLUDE))
		return parse_include(pr, name);

	if (vcs(name->token, STR_SENTENCE))
		return parse_sent(pr, name);

	eet(name->token, WAS_EXPECTING_PREP_INST_WORD, 0);
	return 0;
}

void check_head(struct NodeToken **head, struct NodeToken *damocles) {
	// printf("c [%p]", c);
	// if (c) {
	// 	printf("[%s]\n", vs(c->token));
	// 	printf("\tc->prev  [%p]\n", c->prev);
	// 	printf("\tpr->head [%p]\n", pr->head);
	// } else
	// 	putchar('\n');
	if (damocles) {
		if (damocles->prev == 0)
			*head = damocles;
		else if (damocles->prev->prev == 0)
			*head = damocles->prev;
	}
}

#define iter(cond, line)                                                       \
	if (code cond) {                                                           \
		line;                                                                  \
		goto try_fill_head_if_empty;                                           \
	}

void pre(struct Prep *pr, struct PList *final_tokens) {
	struct NodeToken *c, *name, *fst, *lst;
	enum TCode code;

	for (c = pr->head; c;) {
		code = c->token->code;
		// printf("doin' %s\n", vs(c->token));

		iter(== SH_QR, eet(c->token, SH_QR_CANT_OCCUR_BY_ITSELF, 0));
		iter(== SH_QL, c = sh_string(c));
		iter(== SHPLS, c = shplus(pr, c));
		iter(!= SHARP, c = try_apply(pr, c)->next);

		fst = c;
		name = next_applyed(pr, c);
		lst = try_parse_sh(pr, name);
		if (!lst) { // in case of STR_INCLUD when include
			c = new_included_head;
			new_included_head = 0;
			goto try_fill_head_if_empty;
		}

		// - cut off statemnt nodes so it wont be in final_tokens
		c = cut_off_inclusive(fst, lst); // its already next token in here
		if (pr->head == fst) // fst is freed but pointer is just a value
			pr->head = c;	 // in case if its first and tokens decapitated

	try_fill_head_if_empty:
		check_head(&pr->head, c);
	}

	// collect tokens
	// printf("pr->head [%p]\n", pr->head);
	for (c = pr->head; c; c = c->next)
		plist_add(final_tokens, c->token);
}
