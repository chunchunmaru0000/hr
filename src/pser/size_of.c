#include "pser.h"

int unsafe_size_of_struct(struct BList *name) {
	struct Inst *declare_struct;
	struct Token *name_token;

	for (u32 i = 0; i < parsed_structs->size; i++) {
		declare_struct = plist_get(parsed_structs, i);
		name_token = plist_get(declare_struct->os, DCLR_STRUCT_NAME);

		if (sc((char *)name->st, vs(name_token)))
			return (long)plist_get(declare_struct->os, DCLR_STRUCT_SIZE);
	}
	exit(224);
	return -1;
}

int unsafe_size_of_type(struct TypeExpr *type) {
	enum TypeCode c = type->code;
	long arr_length;

	if (c == TC_TUPLE)
		exit(51);
	if (c == TC_ARR) {
		arr_length = (long)arr_len(type);
		if (arr_length < 0)
			arr_length = 1;
		return arr_length * unsafe_size_of_type(arr_type(type));
	}

	return c == TC_STRUCT ? unsafe_size_of_struct(type->data.name)
		   : c >= TC_VOID ? QWORD
		   : c >= TC_U32  ? DWORD
		   : c >= TC_U16  ? WORD
						  : BYTE;
}

int size_of_struct(struct Pser *p, struct BList *name) {
	uint32_t i;
	long size;
	struct Inst *declare_struct;
	struct Token *name_token;

	for (i = 0; i < parsed_structs->size; i++) {
		declare_struct = plist_get(parsed_structs, i);
		name_token = plist_get(declare_struct->os, DCLR_STRUCT_NAME);

		if (sc((char *)name->st, vs(name_token)))
			goto struct_name_found;
	}
	eet(pser_cur(p), STRUCT_NAME_WASNT_FOUND, (char *)name->st);

struct_name_found:

	size = (long)plist_get(declare_struct->os, DCLR_STRUCT_SIZE);
	return size;
}

int size_of_type(struct Pser *p, struct TypeExpr *type) {
	enum TypeCode c = type->code;
	long arr_length;

	if (c == TC_TUPLE)
		exit(50);
	if (c == TC_ARR) {
		arr_length = (long)arr_len(type);
		if (arr_length < 0)
			arr_length = 1;
		return arr_length * size_of_type(p, arr_type(type));
	}

	return c == TC_STRUCT ? size_of_struct(p, type->data.name)
		   : c >= TC_VOID ? QWORD
		   : c >= TC_U32  ? DWORD
		   : c >= TC_U16  ? WORD
						  : BYTE;
}
