#include "Audio.h"
#include "syntmash.h"

#include <cstring>
#include <cstdlib>
#include <cstdio>

#define COUNTOF(c) (sizeof(c)/sizeof(*(c)))

#define MACHINE_STACK 128

namespace {
const SymaOp test_program[] = {
	{SYMA_OP_PUSH_STATE, 0, 0},
	{SYMA_OP_PUSH_IN, 0, 0},
	{SYMA_OP_ADD, 0, 0},
	{SYMA_OP_FRACT, 0, 0},
	{SYMA_OP_DUP, 0, 0},
	{SYMA_OP_POP_STATE, 0, 0},
	//{SYMA_OP_PUSH, 0, .5f},

	{SYMA_OP_PUSH_STATE, 1, 0},
	{SYMA_OP_PUSH, 0, .5f / 44100.f},
	{SYMA_OP_ADD, 0, 0},
	{SYMA_OP_FRACT, 0, 0},
	{SYMA_OP_DUP, 0, 0},
	{SYMA_OP_POP_STATE, 1, 0},

	{SYMA_OP_PSINE, 0, 0},
	{SYMA_OP_PUSH, 0, 1.f},
	{SYMA_OP_ADD, 0, 0},
	{SYMA_OP_PUSH, 0, .5f},
	{SYMA_OP_MUL, 0, 0},

	{SYMA_OP_PUSH, 0, .1f},
	{SYMA_OP_POW, 0, 0},

	{SYMA_OP_PTRI, 0, 0}
	//{SYMA_OP_POP, 0, 0,}, {SYMA_OP_PSINE, 0, 0}
};
}

Audio::Audio(int samplerate, const char *audio_file)
	: samplerate_(samplerate)
{
	(void)audio_file;
	memset(state_, 0, sizeof(state_));
}

void Audio::synthesize(float *samples, int num_samples, const Timeline &timeline) {
	float input[1] = { 440.f / samplerate_ };
	float stack[16];

	SymaRunContext ctx;
	ctx.program = test_program;
	ctx.program_size = COUNTOF(test_program);
	ctx.stack_size = COUNTOF(stack);
	ctx.stack = stack;
	ctx.input_size = COUNTOF(input);
	ctx.input = input;
	ctx.state = state_;
	ctx.state_size = COUNTOF(state_);

	for (int i = 0; i < num_samples; ++i)
		if (symaRun(&ctx) > 0) {
//			printf("%f %f S ", state_[0], state_[1]);
//			printf("%f %f %f %f %f\n", stack[4], stack[3], stack[2], stack[1], stack[0]);
			samples[i] = stack[0];
		} else
			abort();
}
