#include "tpes.h"
#include <string.h>

extern struct Token *file_to_include;

extern const char *const COLOR_BLACK;
extern const char *const COLOR_RED;
extern const char *const COLOR_LIGHT_RED;
extern const char *const COLOR_GREEN;
extern const char *const COLOR_YELLOW;
extern const char *const COLOR_BLUE;
extern const char *const COLOR_PURPLE;
extern const char *const COLOR_LIGHT_PURPLE;
extern const char *const COLOR_GAY;
extern const char *const COLOR_WHITE;
extern const char *const COLOR_RESET;
extern const char UNDERLINE_CHAR;
#define color_print(c, msg) (printf("%s%s%s", (c), (msg), COLOR_RESET))
uint32_t get_utf8_chars_to_pos(const char *str, int col);

struct Tzer {
	struct Fpfc *f;
	struct Pos *p;
	size_t pos;

	char *code;
	uint32_t clen;
};

enum ExtraType {
	ET_NONE,
	ET_INT,
};

struct ErrorInfo {
	struct Token *t;
	const char *msg;
	const char *sgst;

	void *extra;
	enum ExtraType extra_type;
};
struct ErrorInfo *new_error_info(struct Token *t, const char *const msg,
								 const char *const sgst);

struct Tzer *new_tzer(char *);
struct Token *new_token(struct Tzer *);
struct PList *tze(struct Tzer *, long);
void full_free_token(struct Token *t);
void full_free_token_without_pos(struct Token *t);
void print_source_line(struct Pos *, const char *const, char *);
void ee(struct Pos *, const char *const);
void et(struct Token *t, const char *const msg, const char *const sgst);
void eet(struct Token *t, const char *const msg, const char *const sgst);
#define etei(ei) (et((ei)->t, (ei)->msg, (ei)->sgst))
