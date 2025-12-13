#include <stdint.h>
#include <stdlib.h>

#define uc unsigned char
#define u8 unsigned char
#define u16 unsigned short
#define u32 unsigned int
#define u64 unsigned long

#define constr const char *const
#define BYTE 1
#define WORD 2
#define DWORD 4
#define QWORD 8
#define XWORD 16 // эбайт
#define YWORD 32 // юбайт
#define ZWORD 64 // ябайт

#define is_in_word(sz)                                                         \
	((sz == BYTE) || (sz == WORD) || (sz == DWORD) || (sz == QWORD))

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
uint32_t plist_cut(struct PList *l);
#define plist_clear(l) ((l)->size = 0)
void plist_re(struct PList *l);
void plist_clear_items_free(struct PList *);
void plist_free_items_free(struct PList *);
#define p_last(l) (plist_get((l), (l)->size - 1))
void plat(struct PList *, void **, uint32_t);
#define plat_plist(l, o) (plat((l), (o)->st, (o)->size))

struct BList {
	uc *st; // start
	uint32_t cap_pace;
	uint32_t cap;
	uint32_t size;
};

struct BList *new_blist(uint32_t cap_pace);
struct BList *blist_from_str(char *str, u32 str_len);
struct BList *copy_blist_from_str(char *str);
struct BList *zero_term_blist(struct BList *l);
uint32_t blist_add(struct BList *, uc);
uint32_t blist_cut(struct BList *);
uc blist_get(struct BList *, uint32_t);
uc blist_set(struct BList *, uint32_t, uc);
void blat(struct BList *, uc *, uint32_t);
#define badd_str(l, str) (blat((l), (uc *)(str), strlen((str))))
#define blat_blist(l, o) (blat((l), (o)->st, (o)->size))
#define blist_clear(l) ((l)->size = 0)
void blist_clear_free(struct BList *);
void blist_print(struct BList *);
void blist_add_set(struct BList *, uc, long *, size_t);
struct BList *copy_str(struct BList *src);
struct BList *copy_blist(struct BList *l);
struct BList *int_to_hex_str(long num);
struct BList *int_to_str(long num);
struct BList *real_to_str(double num);
#define b_last(l) (blist_get((l), (l)->size - 1))

#define forever while (1)
#define loa(arr) (sizeof((arr)) / sizeof((arr)[0]))
// String Compare
#define sc(str1, str2) (strcmp((str1), (str2)) == 0)
// View String
#define vs(t) ((char *)(t)->view->st)
// View Compare
#define vc(t1, t2) (sc(vs((t1)), vs((t2))))
// View Compare String
#define vcs(t, str) (sc(vs((t)), (str)))
// String String
#define ss(t) ((char *)(t)->str->st)
// BList String
#define bs(l) ((char *)(l)->st)
// BList Compare
#define bc(l0, l1) (sc(bs(l0), bs(l1)))

#define foreach_begin(item, items)                                             \
	for (i = 0; i < items->size; i++) {                                        \
		item = plist_get(items, i);
#define foreach_by(count, item, items)                                         \
	for ((count) = 0; (count) < items->size; (count)++) {                      \
		item = plist_get(items, (count));
#define foreach_end }

#define exch(var1, var2, tmp) (tmp) = (var1), (var1) = (var2), (var2) = (tmp)
