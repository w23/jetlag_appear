#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>

#define MSG(...) \
	do { \
		printf("%s:%d: ", __FILE__, __LINE__); \
		printf(__VA_ARGS__); \
		printf("\n"); \
	} while (0)

typedef struct ExpNode {
	enum {
		ExpNode_Imm,
		ExpNode_Var,
		ExpNode_Op
	} type;
	struct ExpNode *parent;
	struct ExpNode *children[3];
	int num_children;
} ExpNode;

typedef struct {
	const char *str, *str_end;
	ExpNode *nodes;
	int num_nodes;
} ExpParse;

typedef enum {
	ExpErrorType_None,
	ExpErrorType_InvalidToken,
	ExpErrorType_UnexpectedToken,
	ExpErrorType_TooManyNodes,
} ExpErrorType;

typedef struct {
	ExpErrorType error;
	const char *at;
} ExpError;

typedef enum {
	ExpToken_Error = 0,
	ExpToken_End   = (1<<1),
	ExpToken_Imm   = (1<<2),
	ExpToken_Name  = (1<<3),
	ExpToken_Op    = (1<<4),
	ExpToken_Bra   = (1<<5),
	ExpToken_Ket   = (1<<6),
	ExpToken_Comma = (1<<7),
} ExpTokenType;

typedef struct {
	ExpTokenType type;
	const char *begin, *end;
	float imm;
} ExpToken;

static ExpToken getNextToken(ExpParse *ctx) {
	ExpToken tok;
	tok.type = ExpToken_End;

	while (ctx->str < ctx->str_end && isspace(*ctx->str)) ++ctx->str;

	if (ctx->str >= ctx->str_end)
		return tok;

	tok.type = ExpToken_Error;
	tok.begin = ctx->str;
	tok.end = tok.begin + 1;
	const char c = *tok.begin;

#define TOKEN_CHAR(ch, tok_type) \
	if (c == ch) { \
		tok.type = tok_type; \
	} else

	TOKEN_CHAR('(', ExpToken_Bra)
	TOKEN_CHAR(')', ExpToken_Ket)
	TOKEN_CHAR('+', ExpToken_Op)
	TOKEN_CHAR('-', ExpToken_Op)
	TOKEN_CHAR('*', ExpToken_Op)
	TOKEN_CHAR('/', ExpToken_Op)
	TOKEN_CHAR('^', ExpToken_Op)
	TOKEN_CHAR(',', ExpToken_Comma)
	if (isdigit(c)) {
		tok.type = ExpToken_Imm;
		tok.imm = strtof(tok.begin, (char**)&tok.end);
	} else 
	if (isalpha(c)) {
		tok.type = ExpToken_Name;
		while (tok.end < ctx->str_end) {
			const char c = *tok.end;
			if (!isalnum(c) && c != '_')
				break;
			++tok.end;
		}
	}

	ctx->str = tok.end;
	return tok;
}

static ExpNode *buildTree(ExpNode *tail) {
	for (;;) {
	}

	return NULL;
}

typedef struct ExpSubexpressionContext {
	ExpParse *parse;
	ExpError *error;
	struct {
		ExpTokenType last_token_type;
		ExpNode *root_node;
	} result;

	ExpNode *node_tail;

	ExpToken prev;

	int expected_tokens;

} ExpSubexpressionContext;

static int addNode(ExpSubexpressionContext *ctx, ExpToken *token) {
}

static ExpNode *allocNode(ExpSubexpressionContext *ctx) {
}

static int parseSubexpression(ExpSubexpressionContext *ctx);

static int parseValue(ExpSubexpressionContext *ctx) {
	ExpNode *node = NULL;
	int change_sign = 0;

	const char *first_token_begin = NULL;

	for (;;) {
		const ExpToken tok = getNextToken(ctx->parse);
		ctx->result.last_token_type = tok.type;

		if (!first_token_begin)
			first_token_begin = tok.begin;

		switch (tok.type) {
		case ExpToken_Error:
			ctx->error->error = ExpErrorType_InvalidToken;
			ctx->error->at = tok.begin;
			return 0;
		case ExpToken_Bra:
			// if just (
			// if func(
			if (!parseSubexpression(ctx))
				return 0;
			if (ctx->result.last_token_type == ExpToken_Ket) {

			} else
			if (ctx->result.last_token_type == ExpToken_Comma) {
				break;
			}
			
			ctx->error->error = ExpErrorType_UnexpectedToken;
			ctx->error->at = ctx->parse->str;
			return 0;
		case ExpToken_Op:
			if (tok.begin[0] == '-') {
				change_sign = 1;
				break;
			}
		default:
			ctx->error->error = ExpErrorType_UnexpectedToken;
			ctx->error->at = tok.begin;
			return 0;
		}

	ExpNode *node = allocNode(ctx);
	if (!node) {
		ctx->error->error = ExpErrorType_TooManyNodes;
		ctx->error->at = ctx->parse->str;
		return 0;
	}

	}
}

static int parseSubexpression(ExpSubexpressionContext *ctx) {
	ctx->result.last_token_type = ExpToken_End;
	ctx->result.root_node = NULL;
	ctx->node_tail = NULL;
	ctx->expected_tokens = ExpToken_Imm | ExpToken_Bra | ExpToken_Name;

	ExpNode *node = NULL;

	for (int i = 0;; ++i) {
		const ExpToken tok = getNextToken(ctx->parse);
		ctx->result.last_token_type = tok.type;

		if (tok.type == ExpToken_Error) {
			ctx->error->error = ExpErrorType_InvalidToken;
			ctx->error->at = tok.begin;
			return 0;
		}

		MSG("%ld: token %d name '%.*s'",
			tok.begin - ctx->parse->str,
			tok.type,
			(int)(tok.end - tok.begin),
			tok.begin);

		if (tok.type == ExpToken_End || tok.type == ExpToken_Comma || tok.type == ExpToken_Ket) {
			ctx->result.root_node = buildTree(ctx->node_tail);
			break;
		}

		if (i)
			if (!parseOp(ctx))
				return 0;

		if (!parseValue(ctx))
			return 0;
	}

	return 1;
}

int expParse(ExpParse *parse, ExpError *error) {
	return 0;
}

static void parseString(const char *str) {
	MSG("Parse string '%s':", str);
#define MAX_NODES 256
	ExpNode nodes[MAX_NODES];
	ExpParse parse;
	parse.str = str;
	parse.str_end = parse.str + strlen(str);
	parse.nodes = nodes;
	parse.num_nodes = MAX_NODES;
	
	ExpError error;
	if (!expParse(&parse, &error)) {
		MSG("Error at %ld ('%s'): %d", error.at - str, error.at, error.error);
	}

	MSG("Done\n");
}

int main(int argc, char *argv[]) {
	if (argc < 2) {
		parseString("a");
		parseString("0");
		parseString("-3");
		parseString("1 + b");
		parseString("a + -b * 3");
		parseString("(a + b) * 3");
		parseString("sin(1)");
		parseString("a * sin(1 * b + c) + d");
		parseString("clamp(0., 4. / 5 * e, a * sin(1 * b + c) + d) - 8 * 3 ^ 4");
	}

	for (int i = 1; i < argc; ++i)
		parseString(argv[i]);

	return 0;
}
