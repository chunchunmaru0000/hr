#include <stdint.h>
#include <stdlib.h>

#define uc unsigned char
#define BYTE 1
#define WORD 2
#define DWORD 4
#define QWORD 8
#define XWORD 16 // эбайт
#define YWORD 32 // юбайт
#define ZWORD 64 // ябайт

struct PList {
	void **st; // start
	uint32_t cap_pace;
	uint32_t cap;
	uint32_t size;
};

struct PList *new_plist(uint32_t); // cap pace
uint32_t plist_add(struct PList *, void *);
void *plist_get(struct PList *, uint32_t);
void *plist_set(struct PList *, uint32_t, void *);
void plist_free(struct PList *);
#define plist_clear(l) ((l)->size = 0)
void plist_re(struct PList *l);
void plist_clear_items_free(struct PList *);
void plist_free_items_free(struct PList *);

struct BList {
	uc *st; // start
	uint32_t cap_pace;
	uint32_t cap;
	uint32_t size;
};

struct BList *new_blist(uint32_t cap_pace);
struct BList *blist_from_str(char *str, uint32_t str_len);
void convert_blist_to_blist_from_str(struct BList *l);
uint32_t blist_add(struct BList *, uc);
uint32_t blist_cut(struct BList *);
uc blist_get(struct BList *, uint32_t);
uc blist_set(struct BList *, uint32_t, uc);
void blat(struct BList *, uc *, uint32_t);
#define blat_blist(l, o) (blat((l), (o)->st, (o)->size))
#define blist_clear(l) ((l)->size = 0)
void blist_clear_free(struct BList *);
void blist_print(struct BList *);
void blist_add_set(struct BList *, uc, uint32_t *, size_t);
struct BList *copy_str(struct BList *src);
struct BList *copy_blist(struct BList *l);
