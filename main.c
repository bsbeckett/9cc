#include "9cc.h"

// Input program
char *user_input;

// Current token
Token *token;

// Program nodes
Node *code[100];

int main(int argc, char **argv) {
	if (argc != 2) {
		fprintf(stderr, "%s: invalid number of arguments\n", argv[0]);
		return 1;
	}

	// Tokenize and parse the input
	user_input = argv[1];
	token = tokenize();
	parse();	
    codegen();

	return 0;
}
