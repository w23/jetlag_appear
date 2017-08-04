#include "syntmash.h"
#include "common.h"

#include <stdlib.h>
#include <string.h>

#define MAX_STATE_VARS 32
#define MAX_STATE_VAR_NAME_LENGTH 16

typedef struct {
	SymaOp *ops;
	int ops_count, ops_buffer;

	struct {
		char name[MAX_STATE_VAR_NAME_LENGTH];
		enum {
			StateVar_Float,
			StateVar_Ringbuf
		} type;
		int pos;
		int buffer_size;
	} state_vars[MAX_STATE_VARS];
	int num_state_vars;
	int state_count;
} AsmContext;

static int addOp(AsmContext *ctx, SymaOp op) {
	if (ctx->ops_count >= ctx->ops_buffer) {
		MSG("Parsing error: too many ops");
		return -1;
	}

	ctx->ops[ctx->ops_count++] = op;
	return 0;
}

static int commonOp(const ParserCallbackParams *params) {
	SymaOp op;
	op.opcode = params->line_param0;

	for (int i = 0; i < params->num_args; ++i) {
		switch (params->args[i].type) {
			case PAF_Int:
				op.imm[i].i = params->args[i].value.i;
				break;
			case PAF_Float:
				op.imm[i].f = params->args[i].value.f;
				break;
			default:
				MSG("Unexpected arg %d type %d", i, params->args[i].type);
				return -1;
		}
	}

	return addOp(params->userdata, op);
}

static int stateVar(const ParserCallbackParams *params) {
	AsmContext *ctx = params->userdata;
	if (ctx->num_state_vars == MAX_STATE_VARS) {
		MSG("Too many state vars: %d", ctx->num_state_vars);
		return -1;
	}

	if (params->args[0].slen >= MAX_STATE_VAR_NAME_LENGTH) {
		MSG("Var name '%s' (%d) is too long (%d)",
				params->args[0].slen, params->args[0].s, MAX_STATE_VAR_NAME_LENGTH);
		return -1;
	}

	memcpy(ctx->state_vars[ctx->num_state_vars].name, params->args[0].s, params->args[0].slen + 1);

	if (params->line_param0 == 0) {
		ctx->state_vars[ctx->num_state_vars].type = StateVar_Float;
		ctx->state_vars[ctx->num_state_vars].pos = ctx->state_count++;
	} else if (params->line_param0 == 1) {
		ctx->state_vars[ctx->num_state_vars].type = StateVar_Ringbuf;
		ctx->state_vars[ctx->num_state_vars].pos = ctx->state_count;
		ctx->state_vars[ctx->num_state_vars].buffer_size = params->args[1].value.i;
		ctx->state_count += 2 + params->args[1].value.i;
	}

	++ctx->num_state_vars;
	return 0;
}

static int stateOp(const ParserCallbackParams *params) {
	const AsmContext *ctx = params->userdata;

	SymaOp op;
	op.opcode = params->line_param0;

	for (int i = 0; i < ctx->num_state_vars; ++i) {
		if (strcmp(ctx->state_vars[i].name, params->args[0].s) == 0) {
			if (ctx->state_vars[i].type != StateVar_Float) {
				MSG("State var '%s' is not float", ctx->state_vars[i].name);
				return -1;
			}
			op.imm[0].i = ctx->state_vars[i].pos;
			return addOp(params->userdata, op);
		}
	}

	MSG("State var '%s' is not declared", params->args[0].s);
	return -1;
}

static const ParserLine line_table[] = {
	{"state_var", 0, 1, 4, {PAF_String, PAF_String, PAF_String, PAF_String}, stateVar},
	{"state_ring", 1, 2, 2, {PAF_String, PAF_Int}, stateVar},
	// TODO map inputs
	{"push", SYMA_OP_PUSH, 1, SYMA_MAX_OP_IMM, {PAF_Float}, commonOp},
	{"pop", SYMA_OP_POP, 0, 1, {PAF_Int}, commonOp},
	{"pushi", SYMA_OP_PUSH_IN, 1, 1, {PAF_Int}, commonOp},
	{"pushs", SYMA_OP_PUSH_STATE, 1, 1, {PAF_Var}, stateOp},
	{"pops", SYMA_OP_POP_STATE, 1, 1, {PAF_Var}, stateOp},
	{"dup", SYMA_OP_DUP, 0, 0, {PAF_None}, commonOp},
	{"add", SYMA_OP_ADD, 0, 0, {PAF_None}, commonOp},
	{"padd", SYMA_OP_PADD, 0, 0, {PAF_Float}, commonOp},
	{"mul", SYMA_OP_MUL, 0, 0, {PAF_Float}, commonOp},
	{"psine", SYMA_OP_PSINE, 0, 0, {PAF_Float}, commonOp},
	{"ptri", SYMA_OP_PTRI, 0, 0, {PAF_Float}, commonOp},
	{"fract", SYMA_OP_FRACT, 0, 0, {PAF_Float}, commonOp},
	{"pow", SYMA_OP_POW, 0, 0, {PAF_Float}, commonOp},
	{"paddst", SYMA_OP_PADDST, 1, 1, {PAF_Var}, stateOp},
	{"mtodp", SYMA_OP_MTODP, 0, 0, {PAF_Int}, commonOp},
	{"noise", SYMA_OP_NOISE, 0, 0, {PAF_Int}, commonOp},
	{"madd", SYMA_OP_MADD, 0, 0, {0}, commonOp},
	{"maddi", SYMA_OP_MADDI, 2, 2, {PAF_Float, PAF_Float}, commonOp},
	{"div", SYMA_OP_DIV, 0, 0, {0}, commonOp},
	{"pushdpfreq", SYMA_OP_PUSHDPFREQ, 1, 1, {PAF_Float}, commonOp},
	{"mix", SYMA_OP_MIX, 0, 0, {0}, commonOp},
	{"clamp", SYMA_OP_CLAMP, 0, 0, {0}, commonOp},
	{"clampi", SYMA_OP_CLAMPI, 2, 2, {PAF_Float, PAF_Float}, commonOp},
	{"swap", SYMA_OP_SWAP, 0, 0, {0}, commonOp},
	{"stepi", SYMA_OP_STEPI, 1, 1, {PAF_Float}, commonOp},
	{"rdivi", SYMA_OP_RDIVI, 1, 1, {PAF_Float}, commonOp},
	{"rot", SYMA_OP_ROT, 1, 1, {PAF_Int}, commonOp},
	{"sub", SYMA_OP_SUB, 0, 0, {0}, commonOp},
	{"min", SYMA_OP_MIN, 0, 0, {0}, commonOp},
	{"max", SYMA_OP_MAX, 0, 0, {0}, commonOp},
};

int symasmCompile(SymaRunContext *ctx_inout, const char *source) {
	AsmContext asm_context;
	ParserTokenizer parser;

	memset(&parser, 0, sizeof(parser));
	parser.ctx.line = source;
	parser.ctx.prev_line = 0;
	parser.ctx.line_number = 0;
	parser.line_table = line_table;
	parser.line_table_length = COUNTOF(line_table);
	parser.userdata = &asm_context;

	memset(&asm_context, 0, sizeof(asm_context));
	asm_context.ops = (SymaOp*)ctx_inout->program;
	asm_context.ops_buffer = ctx_inout->program_size;

	for (;;) {
		const int result = parserTokenizeLine(&parser);
		if (result == Tokenize_End)
			break;

		if (result != Tokenize_Parsed) {
			MSG("Parsing error: %d at line %d",
				result, parser.ctx.line_number);
			return 0;
		}

	}

	ctx_inout->program_size = asm_context.ops_count;
	ctx_inout->state_size = asm_context.state_count;
	return 1;
}
