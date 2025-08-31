#include "prep.h"
#include <stdio.h>

const char *const EXPECTED_STR_AS_AN_INCLUDE_PATH =
	"После инструкции препроцессора '#влечь' ожидалась строка содержащая путь "
	"файла.";

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

struct NodeToken *get_included_tail(struct NodeToken *included_head) {
	struct NodeToken *included_tail;

	while (included_head->next && included_head->token->code != EF)
		included_head = included_head->next;

	included_tail = included_head->prev;
	full_free_node_token(included_head); // free EF node token
	// included_tail->next = 0; // could included_tail be 0?

	return included_tail;
}

struct Token *file_to_include = 0;
struct NodeToken *new_included_head = 0;
struct NodeToken *parse_include(struct NodeToken *c) {
	struct NodeToken *fst = c->prev, *lst = c->next, *path_name = lst;
	struct NodeToken *included_head, *included_tail;

	struct BList *path_to_include = try_include_path(path_name->token);
	if (!path_to_include) // ALREADY_INCLUDED
		goto _defer_just_ret_with_no_include;

	file_to_include = path_name->token;

	included_head = get_included_head(path_to_include);
	if (included_head->token->code == EF) {
		full_free_node_token(included_head);
		goto _defer_just_ret_with_no_include;
	}
	file_to_include = 0;
	// included_tail cant be 0 cuz if its then
	// included_head is EF that is handled above
	included_tail = get_included_tail(included_head);

	c = cut_off_inclusive(c, lst); // cut off влечь and path_name

	free(fst->token->p);
	fst->token->p = 0;
	new_included_head = replace_inclusive(fst, included_head, included_tail);

	return 0;

_defer_just_ret_with_no_include:
	file_to_include = 0;
	return lst;
}

struct NodeToken *gen_node_tokens(struct PList *tokens) {
	struct NodeToken *head; // = malloc(sizeof(struct NodeToken));
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
