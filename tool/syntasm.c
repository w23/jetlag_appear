#include "syntmash.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define MAX_TOKEN_LENGTH 16
#define MAX_LINE_TOKENS 2

typedef struct {
	const char *line, *prev_line;
	int line_number;
	struct {
		char str[MAX_TOKEN_LENGTH];
	} token[MAX_LINE_TOKENS];
	int tokens;
	enum {
		Status_Parsed,
		Status_End,
		Status_TokenTooLong,
		Status_TooManyTokens,
	} status;
} ParserContext;

void tokenizeLine(ParserContext *context) {
	const char *p = context->line;
	context->tokens = 0;
	for (;;) {
		while (*p && isspace(*p) && *p != '\n') ++p;
		if (!*p) {
			context->status = Status_End;
			break;
		}

		if (*p == '\n') {
			++p;
			context->status = Status_Parsed;
			break;
		}

		if (*p == ';') {
			while (*p && *p != '\n') ++p;
			context->status = Status_Parsed;
			break;
		}

		const char *start = p;
		while (*p && isgraph(*p)) ++p;
		const int tok_len = p - start;
		if (tok_len >= MAX_TOKEN_LENGTH) {
			context->status = Status_TokenTooLong;
			break;
		}

		if (context->tokens == MAX_LINE_TOKENS) {
			context->status = Status_TokenTooLong;
			break;
		}

		memcpy(context->token[context->tokens].str, start, tok_len);
		context->token[context->tokens].str[tok_len] = '\0';
		++context->tokens;
	}
	if (*p == '\n') ++p;
	context->line = p;
}

static struct {
	const char *name;
	enum syma_opcode_t opcode;
	int args;
	enum {
		ArgType_Float,
		ArgType_Int,
	} arg_type[MAX_LINE_TOKENS-1];
} opcode_table[] = {
	{"push", SYMA_OP_PUSH, 1, {ArgType_Float}},
	{"pop", SYMA_OP_POP, 0, {ArgType_Float}},
	{"pushi", SYMA_OP_PUSH_IN, 1, {ArgType_Int}},
	{"pushs", SYMA_OP_PUSH_STATE, 1, {ArgType_Int}},
	{"pops", SYMA_OP_POP_STATE, 1, {ArgType_Int}},
	{"dup", SYMA_OP_DUP, 0, {ArgType_Float}},
	{"add", SYMA_OP_ADD, 0, {ArgType_Float}},
	{"padd", SYMA_OP_PADD, 0, {ArgType_Float}},
	{"mul", SYMA_OP_MUL, 0, {ArgType_Float}},
	{"psine", SYMA_OP_PSINE, 0, {ArgType_Float}},
	{"ptri", SYMA_OP_PTRI, 0, {ArgType_Float}},
	{"fract", SYMA_OP_FRACT, 0, {ArgType_Float}},
	{"pow", SYMA_OP_POW, 0, {ArgType_Float}},
};

int symasmCompile(SymaRunContext *ctx_inout, const char *source) {
	ParserContext parser_context;
	parser_context.line = source;
	parser_context.prev_line = 0;
	parser_context.line_number = 0;
	SymaOp *op = (SymaOp*)ctx_inout->program;
	for (;;) {
		tokenizeLine(&parser_context);
		if (parser_context.status == Status_End)
			break;
		if (parser_context.status != Status_Parsed) {
			printf("Parsing error: %d at line %d\n",
				parser_context.status, parser_context.line_number);
			return 0;
		}

		if (parser_context.tokens < 1)
			continue;

#define COUNTOF(a) (sizeof(a)/sizeof(*(a)))
		int token_found = 0;
		for (unsigned long i = 0; i < COUNTOF(opcode_table); ++i)
			if (0 == strcmp(parser_context.token[0].str, opcode_table[i].name)) {
				const int args = parser_context.tokens - 1;
				if (args != opcode_table[i].args) {
					printf("Parsing error: incorrect number of args on line %d\n", parser_context.line_number);
					return 0;
				}

				if (op - ctx_inout->program >= ctx_inout->program_size) {
					printf("Parsing error: too many ops on line %d\n", parser_context.line_number);
					return 0;
				}

				if (args == 1) {
					switch (opcode_table[i].arg_type[0]) {
						case ArgType_Int:
							op->immi = strtol(parser_context.token[1].str, 0, 10);
							break;
						case ArgType_Float:
							op->immf = strtod(parser_context.token[1].str, 0);
							break;
					}
				}

				op->opcode = opcode_table[i].opcode;
				++op;

				token_found = 1;
				break;
			}

		if (!token_found) {
			printf("Parsing error: invalid token %s at line %d\n",
					parser_context.token[0].str, parser_context.line_number);
			return 0;
		}
	}

	ctx_inout->program_size = op - ctx_inout->program;

	return 1;
}
