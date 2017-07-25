#include "syntmash.h"

#include <stdio.h>
#include <math.h>

#define CHECK_STACK_OVERFLOW(n) \
	if (sp + n >= context->stack_size) { \
		printf("Stack overflow @%d, op %d (%d, %f)\n", \
			pc, op->opcode, op->immi, op->immf); \
	}
#define CHECK_STACK_UNDERFLOW(n) \
	if (sp - n < 0) { \
		printf("Stack underflow @%d, op %d (%d, %f)\n", \
			pc, op->opcode, op->immi, op->immf); \
	}

int symaRun(SymaRunContext *context) {
	int sp = -1;
	float *stack = context->stack;

	for (int pc = 0; pc < context->program_size; ++pc) {
		const SymaOp *op = context->program + pc;

		switch (op->opcode) {
		case SYMA_OP_PUSH:
			CHECK_STACK_OVERFLOW(1);
			stack[++sp] = op->immf;
			break;
		case SYMA_OP_POP:
			CHECK_STACK_UNDERFLOW(1);
			--sp;
			break;
		case SYMA_OP_PUSH_IN:
			CHECK_STACK_OVERFLOW(1);
			if (op->immi < 0 || op->immi >= context->input_size) {
				printf("Input out-of-bounds: %d (%d)\n", op->immi, context->input_size);
				return 0;
			}
			stack[++sp] = context->input[op->immi];
			break;
		case SYMA_OP_PUSH_STATE:
			CHECK_STACK_OVERFLOW(1);
			if (op->immi < 0 || op->immi >= context->state_size) {
				printf("State out-of-bounds: %d (%d)\n", op->immi, context->state_size);
				return 0;
			}
			stack[++sp] = context->state[op->immi];
			break;
		case SYMA_OP_POP_STATE:
			CHECK_STACK_UNDERFLOW(1);
			if (op->immi < 0 || op->immi >= context->state_size) {
				printf("State out-of-bounds: %d (%d)\n", op->immi, context->state_size);
				return 0;
			}
			context->state[op->immi] = stack[sp--];
			break;
		case SYMA_OP_DUP: {
				CHECK_STACK_OVERFLOW(1);
				const float v = stack[sp];
				stack[++sp] = v;
				break;
			}
		case SYMA_OP_ADD: {
				if (sp < 1) {
					printf("Stack too shallow: %d\n", sp);
					return 0;
				}
				const float v = stack[sp];
				stack[--sp] += v;
				break;
			}
		case SYMA_OP_PADD: {
				if (sp < 1) {
					printf("Stack too shallow: %d\n", sp);
					return 0;
				}
				const float v = stack[sp];
				stack[--sp] += v;
				stack[sp] = fmodf(stack[sp], 1.f);
				break;
			}
		case SYMA_OP_MUL: {
				if (sp < 1) {
					printf("Stack too shallow: %d\n", sp);
					return 0;
				}
				const float v = stack[sp];
				stack[--sp] *= v;
				break;
			}
		case SYMA_OP_PSINE:
			if (sp < 0) {
				printf("Stack too shallow: %d\n", sp);
				return 0;
			}
			stack[sp] = sinf(3.1415927f * 2.f * stack[sp]);
			break;
		case SYMA_OP_PTRI: {
			if (sp < 1) {
				printf("Stack too shallow: %d\n", sp);
				return 0;
			}
			const float v = stack[sp-1], p = stack[sp];
			stack[--sp] = ((v < p) ? 1.f - v / p : (v - p) / (1.f - p)) * 2.f - 1.f;
			break;
		}
		case SYMA_OP_FRACT:
			if (sp < 0) {
				printf("Stack too shallow: %d\n", sp);
				return 0;
			}
			stack[sp] = fmodf(stack[sp], 1.f);
			break;
		case SYMA_OP_POW:
			if (sp < 1) {
				printf("Stack too shallow: %d\n", sp);
				return 0;
			}
			const float v = stack[sp-1], p = stack[sp];
			stack[--sp] = powf(v, p);
			break;
		default:
			printf("Invalid opcode: %d\n", op->opcode);
			return 0;
		}
	}

	return 1;
}
