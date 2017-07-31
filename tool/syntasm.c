#include "syntmash.h"
#include "common.h"

#include <stdlib.h>
#include <string.h>

static struct {
	const char *name;
	enum syma_opcode_t opcode;
	int args;
	enum {
		ArgType_Float,
		ArgType_Int,
	} arg_type[1];
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
		parseLine(&parser_context);
		if (parser_context.status == Status_End)
			break;
		if (parser_context.status != Status_Parsed) {
			MSG("Parsing error: %d at line %d",
				parser_context.status, parser_context.line_number);
			return 0;
		}

		if (parser_context.tokens < 1)
			continue;

		int token_found = 0;
		for (unsigned long i = 0; i < COUNTOF(opcode_table); ++i)
			if (0 == strcmp(parser_context.token[0].str, opcode_table[i].name)) {
				const int args = parser_context.tokens - 1;
				if (args != opcode_table[i].args) {
					MSG("Parsing error: incorrect number of args on line %d", parser_context.line_number);
					return 0;
				}

				if (op - ctx_inout->program >= ctx_inout->program_size) {
					MSG("Parsing error: too many ops on line %d", parser_context.line_number);
					return 0;
				}

				if (args == 1) {
					switch (opcode_table[i].arg_type[0]) {
						case ArgType_Int:
							op->immi = strtol(parser_context.token[1].str, 0, 10);
							break;
						case ArgType_Float:
							op->immf = (float)strtod(parser_context.token[1].str, 0);
							break;
					}
				}

				op->opcode = opcode_table[i].opcode;
				++op;

				token_found = 1;
				break;
			}

		if (!token_found) {
			MSG("Parsing error: invalid token %s at line %d",
					parser_context.token[0].str, parser_context.line_number);
			return 0;
		}
	}

	ctx_inout->program_size = op - ctx_inout->program;

	return 1;
}
