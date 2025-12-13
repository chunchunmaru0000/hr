#include "prep.h"
#include <stdio.h>

constr EXPECTED_STR_AS_AN_INCLUDE_PATH_OR_PAR_L =
	"После инструкции препроцессора '#влечь' ожидалась строка содержащая путь "
	"файла, или открывающая скобка с последующими выражениями путей и "
	"закрывающей скобкой.";
constr EXPECTED_STR_AS_AN_INCLUDE_PATH =
	"После инструкции препроцессора '#влечь' ожидалась строка содержащая путь "
	"файла.";
constr ALREADY_INCLUDED = "Файл уже однажды включен.";
constr EXPECTED_INCLUDES_CLOSE_PAR =
	"Встречен конец файла, а скобка так и не была закрыта.";

struct BList *get_dir(struct BList *path) {
	int i;
	path = copy_str(path);

	if (path->size)
		for (i = path->size - 1; i >= 0; i--) {
			if (path->st[i] == '/')
				break;
			if (path->size)
				path->size--, path->st[i] = 0;
		}
	return path;
}

struct PList *included_dirs;
struct BList *try_include_path(struct Token *path) {
	struct BList *included_path, *loop_path;
	uint32_t i;

	if (!included_dirs) {
		included_dirs = new_plist(8);
		plist_add(included_dirs, zero_term_blist(copy_blist_from_str("")));
	}

	included_path = copy_str(p_last(included_dirs));
	blat_blist(included_path, path->str);
	zero_term_blist(included_path);

	blist_clear_free(path->str);
	path->str = included_path;

	foreach_begin(loop_path, included_files) {
		if (sc(bs(loop_path), bs(included_path)))
			return 0;
	}
	foreach_end;

	plist_add(included_dirs, get_dir(included_path));
	plist_add(included_files, included_path);

	return included_path;
}

struct NodeToken *get_included_head(struct BList *included_path) {
	struct NodeToken *included_head;
	struct PList *new_tokens;
	struct Tzer *tzer;

	tzer = new_tzer((char *)included_path->st);
	new_tokens = tze(tzer, 1024);
	// printf("# gen_node_tokens in %s with %d num of tokens\n",
	// 	   (char *)included_path->st, new_tokens->size);
	included_head = gen_node_tokens(new_tokens);

	free(tzer);
	plist_free(new_tokens); // here every EF token is lost
	return included_head;
}

struct NodeToken *get_included_tail(struct Prep *pr,
									struct NodeToken *included_head) {
	struct NodeToken *included_tail;

	while (included_head->next && included_head->token->code != EF) {
		//printf("a1.1.1 tail %s\n", vs(included_head->token));
		//included_head = preprocess_token(pr, included_head);
		included_head = included_head->next;
		//printf("a1.1.2 tail %s\n", vs(included_head->token));
	}
	blist_clear_free(p_last(included_dirs)), included_dirs->size--;

	included_tail = included_head->prev;
	included_tail->next = 0;
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
	// TODO: this remove from included_dirs should be not here but after
	// preprocessing included file text
	// blist_clear_free(p_last(included_dirs)), included_dirs->size--;

	if (included_head->token->code == EF) {
		full_free_node_token(included_head);
		return 0;
	}

	return included_head;
}

// fst is #, c is влечь
struct NodeToken *single_include(struct Prep *pr, struct NodeToken *c) {
	struct NodeToken *fst = c->prev, *path_name = c->next;
	struct NodeToken *included_head, *included_tail;

	included_head = try_get_included_head(path_name);
	if (!included_head) {
		blist_clear_free(p_last(included_dirs)), included_dirs->size--;
		return path_name; // need when include empty file
	}
	// included_tail cant be 0 cuz if its then
	// included_head is EF that is handled above
	included_tail = get_included_tail(pr, included_head);

	c = cut_off_inclusive(c, path_name); // cut off влечь and path_name

	free(fst->token->p);
	fst->token->p = 0;
	new_included_head = replace_inclusive(fst, included_head, included_tail);
	new_included_tail = included_tail;

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

		printf("a1\n");
		included_head = try_get_included_head(path_name);
		if (!included_head) {
			blist_clear_free(p_last(included_dirs)), included_dirs->size--;
			continue; // empty file
		}
		printf("a1.1\n");
		included_tail = get_included_tail(pr, included_head);
		printf("a1.2\n");

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

	printf("a2\n");
	c = cut_off_inclusive(par_l->prev, c); // cut off влечь and par l to r
	free(fst->token->p);
	fst->token->p = 0;
	new_included_head = replace_inclusive(fst, includes_head, includes_tail);
	//c->next = included_head;
	//included_head->prev = c;
	//new_included_head = included_head;
	// printf("a2.1 tail %s\n", vs(includes_tail->token));
	// printf("tail->next %p\n", includes_tail->next);
	// new_included_head = replace_inclusive(fst, includes_head, includes_tail);
	// new_included_tail = includes_tail;
	printf("a3\n");

	return 0;
}

struct Token *file_to_include = 0;
struct NodeToken *new_included_head = 0;
struct NodeToken *new_included_tail = 0;
struct NodeToken *parse_include(struct Prep *pr, struct NodeToken *c) {
	struct NodeToken *path_name = tgn(c);

	if (path_name->token->code == PAR_L)
		return multi_include(pr, c);
	if (path_name->token->code == STR)
		return single_include(pr, c);

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
