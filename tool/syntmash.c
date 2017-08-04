#include "syntmash.h"

#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#define CHECK_STACK_OVERFLOW(n) \
	if (sp + n >= context->stack_size) { \
		printf("Stack overflow @%d, op %d (%d, %f)\n", \
			pc, op->opcode, op->imm[0].i, op->imm[0].f); \
	}
#define CHECK_STACK_UNDERFLOW(n) \
	if (sp - n < 0) { \
		printf("Stack underflow @%d, op %d (%d, %f)\n", \
			pc, op->opcode, op->imm[0].i, op->imm[0].f); \
	}

int symaRun(SymaRunContext *context) {
	int sp = -1;
	float *stack = context->stack;

	for (int pc = 0; pc < context->program_size; ++pc) {
		const SymaOp *op = context->program + pc;

		switch (op->opcode) {
		case SYMA_OP_PUSH:
			CHECK_STACK_OVERFLOW(1);
			stack[++sp] = op->imm[0].f;
			break;
		case SYMA_OP_POP:
			CHECK_STACK_UNDERFLOW(0);
			--sp;
			break;
		case SYMA_OP_PUSH_IN:
			CHECK_STACK_OVERFLOW(1);
			if (op->imm[0].i < 0 || op->imm[0].i >= context->input_size) {
				printf("Input out-of-bounds: %d (%d)\n", op->imm[0].i, context->input_size);
				return 0;
			}
			stack[++sp] = context->input[op->imm[0].i];
			break;
		case SYMA_OP_PUSH_STATE:
			CHECK_STACK_OVERFLOW(1);
			if (op->imm[0].i < 0 || op->imm[0].i >= context->state_size) {
				printf("State out-of-bounds: %d (%d)\n", op->imm[0].i, context->state_size);
				return 0;
			}
			stack[++sp] = context->state[op->imm[0].i];
			break;
		case SYMA_OP_POP_STATE:
			CHECK_STACK_UNDERFLOW(1);
			if (op->imm[0].i < 0 || op->imm[0].i >= context->state_size) {
				printf("State out-of-bounds: %d (%d)\n", op->imm[0].i, context->state_size);
				return 0;
			}
			context->state[op->imm[0].i] = stack[sp--];
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
		case SYMA_OP_PADDST:
			if (sp < 0) {
				printf("Stack too shallow: %d\n", sp);
				return 0;
			}
			if (op->imm[0].i < 0 || op->imm[0].i >= context->state_size) {
				printf("State out-of-bounds: %d (%d)\n", op->imm[0].i, context->state_size);
				return 0;
			}
			float *const phase = context->state + op->imm[0].i;
			stack[sp] = *phase = fmodf(*phase + stack[sp], 1.f);
			break;
		case SYMA_OP_MTODP:
			if (sp < 0) {
				printf("Stack too shallow: %d\n", sp);
				return 0;
			}
			stack[sp] = powf(2.f, (stack[sp] - 57.f) / 12.f) * 440.f / context->samplerate;
			break;
		case SYMA_OP_NOISE:
			CHECK_STACK_OVERFLOW(1);
			stack[++sp] = (rng(&context->rng) / (float)UINT32_MAX) * 2.f - 1.f;
			break;

		case SYMA_OP_MADD:
			CHECK_STACK_UNDERFLOW(2);
			stack[sp-2] = stack[sp] * stack[sp-1] + stack[sp-2];
			sp -= 2;
			break;

		case SYMA_OP_MADDI:
			CHECK_STACK_UNDERFLOW(0);
			stack[sp] = stack[sp] * op->imm[0].f + op->imm[1].f;
			break;

		case SYMA_OP_DIV:
			CHECK_STACK_UNDERFLOW(1);
			if (fabs(stack[sp]) > 1e-9)
				stack[sp-1] /= stack[sp];
			else
				stack[sp-1] = 0;
			--sp;
			break;

		case SYMA_OP_PUSHDPFREQ:
			CHECK_STACK_OVERFLOW(1);
			stack[++sp] = op->imm[0].f / context->samplerate;
			break;

		case SYMA_OP_MIX:
			CHECK_STACK_UNDERFLOW(2);
			stack[sp-2] = stack[sp-1] * stack[sp] + stack[sp-2] * (1.f - stack[sp]);
			sp -= 2;
			break;

		case SYMA_OP_CLAMP:
			CHECK_STACK_UNDERFLOW(2);
			if (stack[sp] < stack[sp-2])
				stack[sp-2] = stack[sp];
			else if (stack[sp] > stack[sp-1])
				stack[sp-2] = stack[sp-1];
			sp -= 2;
			break;

		case SYMA_OP_CLAMPI:
			CHECK_STACK_UNDERFLOW(0);
			if (stack[sp] < op->imm[0].f)
				stack[sp] = op->imm[0].f;
			else if (stack[sp] > op->imm[1].f)
				stack[sp] = op->imm[1].f;
			break;

		case SYMA_OP_SWAP: {
			CHECK_STACK_UNDERFLOW(1);
			const float tmp = stack[sp];
			stack[sp] = stack[sp-1];
			stack[sp-1] = tmp;
			break;
		}

		case SYMA_OP_STEPI:
			CHECK_STACK_UNDERFLOW(0);
			stack[sp] = stack[sp] < op->imm[0].f ? 0.f : 1.f;
			break;

		case SYMA_OP_RDIVI:
			CHECK_STACK_UNDERFLOW(0);
			if (fabs(stack[sp]) > 1e-9f)
				stack[sp] = op->imm[0].f / stack[sp];
			else
				stack[sp] = 0;
			break;

		case SYMA_OP_ROT:
			CHECK_STACK_UNDERFLOW(abs(op->imm[0].i));
			if (op->imm[0].i < 0) {
				const float tmp = stack[sp+op->imm[0].i];
				for (int i = op->imm[0].i; i < -1; ++i) {
					stack[sp - i] = stack[sp - i + 1];
				}
				stack[sp] = tmp;
			} else {
				const float tmp = stack[sp];
				for (int i = 0; i < op->imm[0].i-1; ++i) {
					stack[sp - i] = stack[sp - i - 1];
				}
				stack[sp - op->imm[0].i] = tmp;
			}
			break;

		case SYMA_OP_SUB:
			CHECK_STACK_UNDERFLOW(1);
			stack[sp-1] -= stack[sp];
			--sp;
			break;

		case SYMA_OP_MIN:
			CHECK_STACK_UNDERFLOW(1);
			if (stack[sp-1] > stack[sp])
				stack[sp-1] = stack[sp];
			--sp;
			break;

		case SYMA_OP_MAX:
			CHECK_STACK_UNDERFLOW(1);
			if (stack[sp-1] < stack[sp])
				stack[sp-1] = stack[sp];
			--sp;
			break;

		default:
			printf("Invalid opcode: %d\n", op->opcode);
			return 0;
		}
	}

	return 1;
}
