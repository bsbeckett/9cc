#include "9cc.h"
// /////////////////////////////////////
// Tokenizer
// /////////////////////////////////////

extern char *user_input;
extern Token *token;

// Reports an error and exits.
void error(char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	exit(1);
}

void error_at(char *loc, char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);

  int pos = loc - user_input;
  fprintf(stderr, "%s\n", user_input);
  fprintf(stderr, "%*s", pos, "");
  fprintf(stderr, "^ ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

bool consume(char *op) {
	if (token->kind != TK_RESERVED || strlen(op) != token->len ||
		memcmp(token->str, op, token->len)) 
		return false;
	token = token->next;
	return true;
}

Token *consume_ident() {
	if (token->kind != TK_IDENT)
		return NULL;
	Token *ident_token = token;
	token = token->next;
	return ident_token;
}

void expect(char *op) {
	if (token->kind != TK_RESERVED || strlen(op) != token->len ||
		memcmp(token->str, op, token->len))
		error_at(token->str, "expected \"%s\"", op);
	token = token->next;
}

bool isNum() {
	return token->kind == TK_NUM;
}

bool isIdent() {
	return token->kind == TK_IDENT;
}

// Returns true if c is valid as the first character of an identifier
bool is_ident1(char c) {
	return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || c == '_';
}

// Returns true if c is valid as a non-first character of an identifier
bool is_ident2(char c) {
	return is_ident1(c) || ('0' <= c && c <= '9');
}

int expect_number() {
	if (token->kind != TK_NUM)
		error_at(token->str, "expected a number");
	int val = token->val;
	token = token->next;
	return val;
}

bool at_eof() {
	return token->kind == TK_EOF;
}

// Create a new token
Token *new_token(TokenKind kind, Token *cur, char *str, int len) {
	Token *tok = calloc(1, sizeof(Token));
	tok->kind = kind;
	tok->str = str;
	tok->len = len;
	cur->next = tok;
	return tok;
}

bool startswith(char *p, char *q) {
	return memcmp(p, q, strlen(q)) == 0;
}

// Tokenize 'p' and returns new tokens.
Token *tokenize() {
	char *p = user_input;
	Token head;
	head.next = NULL;
	Token *cur = &head;

	while (*p) {
		// Skip whitespace characters
		if (isspace(*p)) {
			p++;
			continue;
		}

		// Multi-letter Operators
		if (startswith(p, "==") || startswith(p, "!=") ||
			startswith(p, "<=") || startswith(p, ">=")) {
			cur = new_token(TK_RESERVED, cur, p, 2);
			p += 2;
			continue;
		}

		// Single-letter Operators
		if (strchr("+-*/()<>=;", *p)) { 
			cur = new_token(TK_RESERVED, cur, p++, 1);
			continue;
		}

		// Single-letter Identifiers
		if (is_ident1(*p)) {
			char *start = p;
			char buffer[100];
			do {
				buffer[p-start] = *p;
				p++;
			} while (is_ident2(*p));
			buffer[p-start] = '\0';
			cur = new_token(TK_IDENT, cur, start, p-start);
			//printf("Local var ident: %s\n", buffer);
			continue;
		}

		// Integer literal
		if (isdigit(*p)) {
			cur = new_token(TK_NUM, cur, p, 0);
			char *q = p;
			cur->val = strtol(p, &p, 10);
			cur->len = p - q;
			continue;
		}

		error_at(p, "invalid token");
	}

	new_token(TK_EOF, cur, p, 0);
	return head.next;
}
