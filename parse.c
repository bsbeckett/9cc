#include "9cc.h"
// ///////////////////////////////////////
// Parser
// ///////////////////////////////////////

extern Node *code[100];
extern LVar *locals;

Node *new_node(NodeKind kind) {
	Node *node = calloc(1, sizeof(Node));
	node->kind = kind;
	return node;
}

Node *new_binary(NodeKind kind, Node *lhs, Node *rhs) {
	Node *node = new_node(kind);
	node->lhs = lhs;
	node->rhs = rhs;
	return node;
}

Node *new_num(int val) {
	Node *node = new_node(ND_NUM);
	node->val = val;
	return node;
}

// Find local variables by name. Return NULL if not found.
LVar *find_lvar(Token *tok) {
	for (LVar *var = locals; var; var = var->next)
		if (var->len == tok->len && !memcmp(tok->str, var->name, var->len))
			return var;
	return NULL;
}

void program();
Node *stmt();
Node *expr();
Node *assign();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();

// program = stmt*
void program() {
	int i = 0;
	while (!at_eof())
		code[i++] = stmt();
	code[i] = NULL;
}

// stmt = expr ";"
Node *stmt() {
	Node *node = expr();
	expect(";");
	return node;
}


// expr = assign
Node *expr() {
	return assign();
}

// assign = equality ("=" assign)?
Node *assign() {
	Node *node = equality();
	if (consume("="))
		node = new_binary(ND_ASSIGN, node, assign());
	return node;
}

// equality = relational ("==" relational | "!=" relational)*
Node *equality() {
	Node *node = relational();
	
	for (;;) {
		if (consume("=="))
			node = new_binary(ND_EQ, node, relational());
		else if (consume("!="))
			node = new_binary(ND_NE, node, relational());
		else
			return node;
	}
}

// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
Node *relational() {
	Node *node = add();

	for (;;) {
		if (consume("<"))
			node = new_binary(ND_LT, node, add());
		else if (consume("<="))
			node = new_binary(ND_LE, node, add());
		else if (consume(">"))
			node = new_binary(ND_LT, add(), node);
		else if (consume(">="))
			node = new_binary(ND_LE, add(), node);
		else
			return node;
	}
}

// add = mul ("+" mul | "-" mul)*
Node *add() {
	Node *node = mul();

	for (;;) {
		if (consume("+"))
			node = new_binary(ND_ADD, node, mul());
		else if (consume("-"))
			node = new_binary(ND_SUB, node, mul());
		else
			return node;
	}
}

// mul = unary ("*" unary | "/" unary)*
Node *mul() {
	Node *node = unary();

	for (;;) {
		if (consume("*"))
			node = new_binary(ND_MUL, node, unary());
		else if (consume("/"))
			node = new_binary(ND_DIV, node, unary());
		else
			return node;
	}
}

// unary = ("+" | "-")? unary
//         | primary
Node *unary() {
	if (consume("+"))
		return unary();
	if (consume("-"))
		return new_binary(ND_SUB, new_num(0), unary());
	return primary();
}

// primary = "(" expr ")" | num
Node *primary() {
	// If the next token is "(", it should be "(" expr ")"
	if (consume("(")) {
		Node *node = expr();
		expect(")");
		return node;
	}
	
	Token *tok = consume_ident();
	if(tok) {
		Node *node = calloc(1, sizeof(Node));
		node->kind = ND_LVAR;

		LVar *lvar = find_lvar(tok);
		if (lvar) {
			node->offset = lvar->offset;
			//printf("Variable found\n");
		} else {
			//printf("Variable DNE\n");
			lvar = calloc(1, sizeof(LVar));
			lvar->next = locals;
			lvar->name = tok->str;
			lvar->len = tok->len;
			// If locals == NULL, then start at 8
			// otherwise add 8 to the previous offset
			lvar->offset = (locals) ? locals->offset + 8 : 8;
			node->offset = lvar->offset;
			locals = lvar;
		}
		//node->offset = (tok->str[0] - 'a' + 1) * 8;

		return node;
	}
	
	// Otherwise it should be a number
	return new_num(expect_number());
}


Node *parse() {
	program();
}
