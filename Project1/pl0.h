#include <stdio.h>
//text

#define NRW        15   // number of reserved words
#define TXMAX      500    // length of identifier table
#define MAXNUMLEN  14     // maximum number of digits in numbers
#define NSYM       16     // maximum number of symbols in array ssym and csym
#define MAXIDLEN   10     // length of identifiers

#define MAXADDRESS 32767  // maximum address
#define MAXLEVEL   32     // maximum depth of nesting block
#define CXMAX      1000    // size of code array

#define MAXSYM     30     // maximum number of symbols  

#define STACKSIZE  1000   // maximum storage

//sympol type
enum symtype
{
	SYM_NULL,
	SYM_IDENTIFIER,
	SYM_NUMBER,
	SYM_PLUS,
	SYM_MINUS,
	SYM_TIMES,
	SYM_SLASH,
	SYM_ODD,
	SYM_EQU,
	SYM_NEQ,
	SYM_LES,
	SYM_LEQ,
	SYM_GTR,
	SYM_GEQ,
	SYM_LPAREN,
	SYM_RPAREN,
	SYM_COMMA,
	SYM_SEMICOLON,
	SYM_PERIOD,
	SYM_BECOMES,
	SYM_BEGIN,
	SYM_END,
	SYM_IF,
	SYM_WHILE,
	SYM_CONST,
	SYM_VAR,
	SYM_PROCEDURE,
	SYM_AND,
	SYM_OR,
	SYM_ELSE, 
	SYM_ELIF, 
	SYM_EXIT, 
	SYM_RETURN, 
	SYM_FOR,
	SYM_SWITCH,
	SYM_CASE,
	SYM_LSQUARE, 
	SYM_RSQUARE,
	SYM_NEG,
	SYM_MOD,
	SYM_BAND,
	SYM_BOR,
	SYM_BXOR,
	SYM_COLON
};

enum idtype
{
	ID_CONSTANT, ID_VARIABLE, ID_PROCEDURE, ID_ARRAY,
	ID_DEFAULTPRO
};

//操作码
enum opcode
{
	LIT, OPR, LOD, STO, CAL, INT, JMP, JPC, LODAR, STOAR, POP, JNE,
	CALL
};

//进一步用来选择C语言中的操作
enum oprcode
{
	OPR_RET, OPR_NEG, OPR_ADD, OPR_MIN,
	OPR_MUL, OPR_DIV, OPR_ODD, OPR_EQU,
	OPR_NEQ, OPR_LES, OPR_LEQ, OPR_GTR,
	OPR_GEQ, OPR_AND, OPR_OR,  OPR_NEGL,
	OPR_MOD, OPR_BAND,OPR_BOR, OPR_BXOR,
	OPR_BECOMES
};

enum funcode
{
	FUN_PRINT,FUN_RANDOM,FUN_CALLSTACK
};

typedef struct
{
	int f; // function code
	int l; // level
	int a; // displacement address
} instruction;

//////////////////////////////////////////////////////////////////////
char* err_msg[] =
{
	/*  0 */    "",
	/*  1 */    "Found '==' when expecting '='.",
	/*  2 */    "There must be a number to follow '='.",
	/*  3 */    "There must be an '=' to follow the identifier.",
	/*  4 */    "There must be an identifier to follow 'const', 'var', or 'procedure'.",
	/*  5 */    "Missing ',' or ';'.",
	/*  6 */    "Incorrect procedure name.",
	/*  7 */    "Statement expected.",
	/*  8 */    "Follow the statement is an incorrect symbol.",
	/*  9 */    "'.' expected.",
	/* 10 */    "';' expected.",
	/* 11 */    "Undeclared identifier.",
	/* 12 */    "Illegal assignment.",
	/* 13 */    "procedure can't be a formal parameter.",
	/* 14 */    "There must be an identifier to follow the 'call'.",
	/* 15 */    "A constant or variable can not be called.",
	/* 16 */    "'end' expected.",
	/* 17 */    "';' or 'end'or ':' expected.",
	/* 18 */    "'begin' expected.",
	/* 19 */    "Incorrect symbol.",
	/* 20 */    "Relative operators expected.",
	/* 21 */    "Procedure identifier can not be in an expression without bracket pair.",
	/* 22 */    "Missing ')'or '('.",
	/* 23 */    "The symbol can not be followed by a factor.",
	/* 24 */    "The symbol can not be as the beginning of an expression.",
	/* 25 */    "The number is too great.",
	/* 26 */    "Illegal procedure declarations.",
	/* 27 */    "The number of the parameter of the procedure is wrong.",
	/* 28 */    "Illegal array declarations.",
	/* 29 */    "Missing ']'.",
	/* 30 */    "Wrong dims.",
	/* 31 */    "",
	/* 32 */    "There are too many levels."
};

//////////////////////////////////////////////////////////////////////
char ch;         // last character read
int  sym;        // last symbol read
char id[MAXIDLEN + 1]; // last identifier read
int  num;        // last number read
int  cc;         // character count
int  ll;         // line length
int  kk;
int  err;
int  cx;         // index of current instruction to be generated.
int  level = 0;
int  tx = 0;//符号表中的条目数
int  formal_para=0;//当前过程形参数目
int fun_code;//内建方法号

char line[80];//存储从文件中取出的一整行字符

instruction code[CXMAX];

//保留字表
char* word[NRW + 1] =
{
	"", /* place holder */
	"begin", "const", "end", "if",
	"odd", "procedure",  "var", "while",
	"else","elif","exit","return","for",
	"switch","case"
};

//与保留字表相对应的记号名
int wsym[NRW + 1] =
{
	SYM_NULL, SYM_BEGIN, SYM_CONST, SYM_END,
	SYM_IF, SYM_ODD, SYM_PROCEDURE, SYM_VAR, SYM_WHILE,
	SYM_ELSE, SYM_ELIF, SYM_EXIT, SYM_RETURN, SYM_FOR,
	SYM_SWITCH,SYM_CASE
};

//与运算符表相对应的记号名
int ssym[NSYM + 1] =
{
	SYM_NULL, SYM_PLUS, SYM_MINUS, SYM_TIMES, 
	SYM_LPAREN, SYM_RPAREN,SYM_LSQUARE,SYM_RSQUARE, 
	SYM_COMMA, SYM_PERIOD, SYM_SEMICOLON,SYM_NEG,SYM_MOD,SYM_BXOR,SYM_COLON
};

//运算符表
char csym[NSYM + 1] =
{
	' ', '+', '-', '*', '(', ')','[',']', ',', '.', ';','!','%','^',':'
};

#define MAXINS   13
//‘指令’集
char* mnemonic[MAXINS] =
{
	"LIT", "OPR", "LOD", "STO", "CAL", "INT", "JMP", "JPC", "LODAR", "STOAR", "POP", "JNE",
	"CALL"
};

typedef struct
{
	char name[MAXIDLEN + 1];
	int  kind;
	int  value;
	int  config[10];
} comtab;

comtab table[TXMAX];//定义了符号表

typedef struct
{
	char  name[MAXIDLEN + 1];
	int   kind;
	short level;
	short address;
	int   config[10];
} mask;

FILE* infile;

// EOF PL0.h
#pragma once
