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

char *user_input;

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

// /////////////////////////////////////
// Tokenizer
// /////////////////////////////////////

bool consume(char op) {
	if (token->kind != TK_RESERVED || token->str[0] != op) 
		return false;
	token = token->next;
	return true;
}

void expect(char op) {
	if (token->kind != TK_RESERVED || token->str[0] != op)
		error_at(token->str, "expected '%c'", op);
	token = token->next;
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
		if  (*p == '+' || *p == '-' || *p == '*' || *p == '/' ||
             *p == '(' || *p == ')') {
			cur = new_token(TK_RESERVED, cur, p++);
			continue;
		}

		// Numerical literal
		if (isdigit(*p)) {
			cur = new_token(TK_NUM, cur, p);
			cur->val = strtol(p, &p, 10);
			continue;
		}

		error_at(p, "Invalid token");
	}

	new_token(TK_EOF, cur, p);
	return head.next;
}

// ///////////////////////////////////////
// Parser
// ///////////////////////////////////////

// Types of AST nodes
typedef enum {
	ND_ADD, // +
	ND_SUB, // -
	ND_MUL, // *
	ND_DIV, // /
	ND_NUM, // Numeric literal
} NodeKind;

typedef struct Node Node;

struct Node {
	NodeKind kind;	// Node type
	Node *lhs;		// Left side
	Node *rhs;		// Right side
	int val;		// Used only when kind is ND_NUM
};

Node *new_node(NodeKind kind, Node *lhs, Node *rhs) {
	Node *node = calloc(1, sizeof(Node));
	node->kind = kind;
	node->lhs = lhs;
	node->rhs = rhs;
	return node;
}

Node *new_node_num(int val) {
	Node *node = calloc(1, sizeof(Node));
	node->kind = ND_NUM;
	node->val = val;
	return node;
}

Node *expr();
Node *primary() {
	// If the next token is "(", it should be "(" expr ")"
	if (consume('(')) {
		Node *node = expr();
		expect(')');
		return node;
	}

	// Otherwise it should be a number
	return new_node_num(expect_number());
}

Node *unary() {
	if (consume('+'))
		return primary();
	if (consume('-'))
		return new_node(ND_SUB, new_node_num(0), primary());
	return primary();
}

Node *mul() {
	Node *node = unary();

	for (;;) {
		if (consume('*'))
			node = new_node(ND_MUL, node, unary());
		else if (consume('/'))
			node = new_node(ND_DIV, node, unary());
		else
			return node;
	}
}

Node *expr() {
	Node *node = mul();

	for (;;) {
		if (consume('+'))
			node = new_node(ND_ADD, node, mul());
		else if (consume('-'))
			node = new_node(ND_SUB, node, mul());
		else
			return node;
	}
}

// ////////////////////////////////
// Code Generator
// ////////////////////////////////
void gen(Node *node) {
	if (node->kind == ND_NUM) {
		printf("  push %d\n", node->val);
		return;
	}

	gen(node->lhs);
	gen(node->rhs);

	printf("  pop rdi\n");
	printf("  pop rax\n");

	switch (node->kind) {
	case ND_ADD:
		printf("  add rax, rdi\n");
		break;
	case ND_SUB:
		printf("  sub rax, rdi\n");
		break;
	case ND_MUL:
		printf("  imul rax, rdi\n");
		break;
	case ND_DIV:
		printf("  cqo\n");
		printf("  idiv rdi\n");
		break;
	}

	printf("  push rax\n");
}
int main(int argc, char **argv) {
	if (argc != 2) {
		fprintf(stderr, "%s: invalid number of arguments\n", argv[0]);
		return 1;
	}

	// Tokenize and parse the input
	user_input = argv[1];
	token = tokenize(user_input);
	Node *node = expr();	

	// Output the first part of the assembly
	printf(".intel_syntax noprefix\n");
	printf(".globl main\n");
	printf("main:\n");

	// Generate code while descending the AST
	gen(node);

	// The value of the entire expression should be at the top of the stack.
	// So, load it into RAX and use it as the return value from the function.
	printf("  pop rax\n");
	printf("  ret\n");
	return 0;
}
