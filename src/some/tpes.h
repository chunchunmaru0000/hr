#include "lsts.h"
#include <stdint.h>

enum Target {
	//	T_Fasm_Linux_64,
	T_Асм_Linux_64,
};

struct Fpfc { // File Path File Code
	const char *path;
	const char *code;
	size_t clen; // code len
};

struct Pos {
	uint32_t line;
	uint32_t col;
	struct Fpfc *f;
};

enum TCode {
	// WORD
	ID, // identifier
	// OTHER
	SLASHN,	 // \n
	COLO,	 // :
	COM,	 // comment
	SLASH,	 // \ slash
	COMMA,	 // ,
	SEP,	 // // separator
	PAR_L,	 // (
	PAR_R,	 // )
	PAR_C_L, // [
	PAR_C_R, // ]
	PAR_T_L, // {
	PAR_T_R, // }
	EXCL,	 // !
	SHARP,	 // #
	EF,		 // end file
	// LITERALS
	INT,  // int literal
	REAL, // float literal
	STR,  // string literal with ""
	// MATH OPS
	INC,	 // ++
	DEC,	 // --
	EQU,	 // =
	PLUS,	 // +
	MINUS,	 // -
	MUL,	 // *
	DIV,	 // /
	MOD,	 // %
	BIT_NOT, // ~
	AMPER,	 // &
	BIT_XOR, // ^
	BIT_OR,	 // |
	AND,	 // &&
	OR,		 // ||
	EQUE,	 // ==
	NEQU,	 // !=
	MORE,	 // >
	LESS,	 // <
	SHL,	 // <<
	SHR,	 // >>
	MOREE,	 // >=
	LESSE,	 // <=
	QUEST,	 // ?

	PLUSE,	  // +=
	MINUSE,	  // -=
	MULE,	  // *=
	DIVE,	  // /=
	SHLE,	  // <<=
	SHRE,	  // >>=
	BIT_ANDE, // &=
	BIT_ORE,  // |=
	BIT_XORE, // ^=
	MODE,	  // %=
	ANDE,	  // &&=
	ORE,	  // ||=
	EQUEE,	  // ===
	NEQUE,	  // !==

	WHOLE_DIV,	  // /:
	FIELD_ARROW,  // ->
	SOBAKA_ARROW, // -. old one was -@
	PIPE_LINE,	  // |>
	DOT,		  // .
	QQ,			  // ??
	CQ,			  // :?
	CC,			  // ::
	DD,			  // ..
	DDD,		  // ...
	DDE,		  // ..=
	LOOP_ARROW,	  // =>

	SHPLS, // #+
	SH_L,  // (#
	SH_R,  // #)
	SH_QL, // #"
	SH_QR, // "#
	SH_OR, // #|
};

struct Token {
	struct BList *view;
	enum TCode code;
	struct Pos *p;
	// literals reserved that may not always be used
	long num;
	double real; // Floating Point Number
	struct BList *str;
};
