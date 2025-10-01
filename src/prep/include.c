#include "prep.h"
#include <stdio.h>

const char *const EXPECTED_STR_AS_AN_INCLUDE_PATH_OR_PAR_L =
	"После инструкции препроцессора '#влечь' ожидалась строка содержащая путь "
	"файла, или открывающая скобка с последующими выражениями путей и "
	"закрывающей скобкой.";
const char *const EXPECTED_STR_AS_AN_INCLUDE_PATH =
	"После инструкции препроцессора '#влечь' ожидалась строка содержащая путь "
	"файла.";
const char *const ALREADY_INCLUDED = "Файл уже однажды включен.";
const char *const EXPECTED_INCLUDES_CLOSE_PAR =
	"Встречен конец файла, а скобка так и не была закрыта.";

struct BList *try_include_path(struct Token *path) {
	struct BList *included_path;
	uint32_t i;

	foreach_begin(included_path, included_files);
	// here compare token str cuz view has "
	if (sc(ss(path), (char *)included_path->st))
		return 0;
	foreach_end;

	included_path = copy_str(path->str);
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

struct NodeToken *get_included_tail(struct NodeToken *included_head) {
	struct NodeToken *included_tail;

	while (included_head->next && included_head->token->code != EF)
		included_head = included_head->next;

	included_tail = included_head->prev;
	full_free_node_token(included_head); // free EF node token
	// included_tail->next = 0; // could included_tail be 0?

	return included_tail;
}

struct NodeToken *try_get_included_head(struct NodeToken *path_name) {
	struct NodeToken *included_head;

	struct BList *path_to_include = try_include_path(path_name->token);
	if (!path_to_include)
		eet(path_name->token, ALREADY_INCLUDED, 0);

	file_to_include = path_name->token;
	included_head = get_included_head(path_to_include);
	file_to_include = 0;

	if (included_head->token->code == EF) {
		full_free_node_token(included_head);
		return 0;
	}

	return included_head;
}

// fst is #, c is влечь
struct NodeToken *single_include(struct NodeToken *c) {
	struct NodeToken *fst = c->prev, *path_name = c->next;
	struct NodeToken *included_head, *included_tail;

	included_head = try_get_included_head(path_name);
	if (!included_head)
		return path_name; // need when include empty file
	// included_tail cant be 0 cuz if its then
	// included_head is EF that is handled above
	included_tail = get_included_tail(included_head);

	c = cut_off_inclusive(c, path_name); // cut off влечь and path_name

	free(fst->token->p);
	fst->token->p = 0;
	new_included_head = replace_inclusive(fst, included_head, included_tail);

	return 0;
}

// fst is #, c is влечь
struct NodeToken *multi_include(struct Prep *pr, struct NodeToken *c) {
	struct NodeToken *fst = c->prev, *par_l = c->next, *path_name = 0;
	struct NodeToken *included_head, *included_tail;
	struct NodeToken *includes_head = 0, *includes_tail;

	for (c = tgn(par_l); c->token->code != PAR_R; c = path_name->next) {
		if (!c || c->token->code == EF)
			eet(par_l->token, EXPECTED_INCLUDES_CLOSE_PAR, 0);
		path_name = try_apply(pr, c);
		if (path_name->token->code != STR)
			eet(path_name->token, EXPECTED_STR_AS_AN_INCLUDE_PATH, 0);

		included_head = try_get_included_head(path_name);
		if (!included_head)
			continue; // empty file
		included_tail = get_included_tail(included_head);

		if (includes_head == 0) {
			includes_head = included_head;
		} else {
			includes_tail->next = included_head;
			included_head->prev = includes_tail;
		}
		includes_tail = included_tail;
	}
	if (path_name == 0)
		return c; // no files to include

	c = cut_off_inclusive(par_l->prev, c); // cut off влечь and par l to r
	free(fst->token->p);
	fst->token->p = 0;
	new_included_head = replace_inclusive(fst, includes_head, includes_tail);
	return 0;
}

struct Token *file_to_include = 0;
struct NodeToken *new_included_head = 0;
struct NodeToken *parse_include(struct Prep *pr, struct NodeToken *c) {
	struct NodeToken *path_name = tgn(c);

	if (path_name->token->code == PAR_L)
		return multi_include(pr, c);
	if (path_name->token->code == STR)
		return single_include(c);

	eet(path_name->token, EXPECTED_STR_AS_AN_INCLUDE_PATH_OR_PAR_L, 0);
	return (void *)-1;
}

struct NodeToken *gen_node_tokens(struct PList *tokens) {
	struct NodeToken *head;
	struct NodeToken *cur = head, *prev = 0;
	uint32_t i;

	for (i = 0; i < tokens->size; i++) {
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
	//         vs(head->token));
	return head;
}
