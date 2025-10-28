#include "../some/tpes.h"
#include <string.h>

extern struct Token *file_to_include;

extern constr COLOR_BLACK;
extern constr COLOR_RED;
extern constr COLOR_LIGHT_RED;
extern constr COLOR_GREEN;
extern constr COLOR_YELLOW;
extern constr COLOR_BLUE;
extern constr COLOR_PURPLE;
extern constr COLOR_LIGHT_PURPLE;
extern constr COLOR_GAY;
extern constr COLOR_WHITE;
extern constr COLOR_RESET;
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
struct Token *new_tok(struct BList *view, enum TCode code, struct Pos *p);

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
struct ErrorInfo *new_error_info(struct Token *t, constr msg, constr sgst);

struct Tzer *new_tzer(char *);
struct Token *new_token(struct Tzer *);
struct PList *tze(struct Tzer *, long);
void full_free_token(struct Token *t);
void full_free_token_without_pos(struct Token *t);
void print_source_line(struct Pos *, constr, char *);
void ee(struct Pos *, constr);
void et(struct Token *t, constr msg, constr sgst);
void eet(struct Token *t, constr msg, constr sgst);
void eet2(struct Token *t0, struct Token *t1, constr msg, constr sgst);
#define etei(ei) (et((ei)->t, (ei)->msg, (ei)->sgst))
