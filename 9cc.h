#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

//
// tokenize.c
//

typedef enum {
	TK_RESERVED,// Reserved operators
	TK_IDENT,	// Identifier
	TK_NUM,		// Numeric literals
	TK_EOF,		// End-of-file marker
} TokenKind;

// Token type
typedef struct Token Token;
struct Token {
	TokenKind kind;	// Token kind
	Token *next;	// Next token
	int val;		// if kind is TK_NUM, its value
	char *str;		// Token string
	int len;		// Token length
};

void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);
bool consume(char *op);
Token *consume_ident();
void expect(char *op);
int expect_number();
bool at_eof();
Token *new_token(TokenKind kind, Token *cur, char *str, int len);
bool startswith(char *p, char *q);
Token *tokenize();

//
// parse.c
//

typedef enum {
  ND_ADD, 	// +
  ND_SUB, 	// -
  ND_MUL, 	// *
  ND_DIV, 	// /
  ND_NEG, 	// unary -
  ND_EQ,  	// ==
  ND_NE,  	// !=
  ND_LT,  	// <
  ND_LE,  	// <=
  ND_ASSIGN,// =
  ND_LVAR,	// Local Variable
  ND_NUM, 	// Integer
} NodeKind;

// AST node type
typedef struct Node Node;
struct Node {
  NodeKind kind; // Node kind
  Node *lhs;     // Left-hand side
  Node *rhs;     // Right-hand side
  int val;       // Used if kind == ND_NUM
  int offset;    // Used if kind == ND_LVAR
};

Node *parse();

//
// codegen.c
//

void codegen();
