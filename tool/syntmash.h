#pragma once

#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

enum syma_opcode_t {
	SYMA_OP_PUSH,
	SYMA_OP_POP,
	SYMA_OP_PUSH_IN,
	SYMA_OP_PUSH_STATE,
	SYMA_OP_POP_STATE,
	SYMA_OP_DUP,
	SYMA_OP_ADD,
	SYMA_OP_PADD,
	SYMA_OP_MUL,
	SYMA_OP_PSINE,
	SYMA_OP_PTRI,
	SYMA_OP_FRACT,
	SYMA_OP_POW,
	SYMA_OP_PADDST,
	SYMA_OP_MTODP,
	SYMA_OP_NOISE,
};

typedef struct {
	enum syma_opcode_t opcode;
	int immi;
	float immf;
} SymaOp;

typedef struct {
	const SymaOp *program;
	int program_size;

	const float *input;
	int input_size;

	float *stack;
	int stack_size;

	float *state;
	int state_size;

	uint64_t rng;
	int samplerate;
} SymaRunContext;

int symaRun(SymaRunContext *context);

#ifdef __cplusplus
}
#endif
