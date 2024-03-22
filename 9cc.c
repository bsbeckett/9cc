#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>

typedef enum {
	TK_RESERVED,// Reserved operators
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
};

// Global variable for tokens
static Token *token;

// Reports an error and exits.
static void error(char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	exit(1);
}

bool consume(char op) {
	if (token->kind != TK_RESERVED || token->str[0] != op)
		return false;
	token = token->next;
	return true;
}

void expect(char op) {
	if (token->kind != TK_RESERVED || token->str[0] != op)
		error("'%c' not found", op);
	token = token->next;
}

int expect_number() {
	if (token->kind != TK_NUM)
		error("Expected a number");
	int val = token->val;
	token = token->next;
	return val;
}

bool at_eof() {
	return token->kind == TK_EOF;
}

// Create a new token
Token *new_token(TokenKind kind, Token *cur, char *str) {
	Token *tok = calloc(1, sizeof(Token));
	tok->kind = kind;
	tok->str = str;
	cur->next = tok;
	return tok;
}

// Tokenize 'p' and returns new tokens.
Token *tokenize(char *p) {
	Token head;
	head.next = NULL;
	Token *cur = &head;

	while (*p) {
		// Skip whitespace characters
		if (isspace(*p)) {
			p++;
			continue;
		}

		// Operators
		if (*p == '+' || *p == '-') {
			cur = new_token(TK_RESERVED, cur, p++);
			continue;
		}

		// Numerical literal
		if (isdigit(*p)) {
			cur = new_token(TK_NUM, cur, p);
			cur->val = strtol(p, &p, 10);
			continue;
		}

		error("Invalid token");
	}

	new_token(TK_EOF, cur, p);
	return head.next;
}

int main(int argc, char **argv) {
	if (argc != 2) {
		fprintf(stderr, "%s: invalid number of arguments\n", argv[0]);
		return 1;
	}

	token = tokenize(argv[1]);

	printf(".intel_syntax noprefix\n");
	printf(".globl main\n");
	printf("main:\n");

	// First token must be a number
	printf("  mov rax, %d\n", expect_number());

	// ... followed by either '+ <number>' or '- <number>'.
	while (!at_eof()) {
		if (consume('+')) {
			printf("  add rax, %d\n", expect_number());
			continue;
		}

		expect('-');
		printf("  sub rax, %d\n", expect_number());
	}

	printf("  ret\n");
	return 0;
}
