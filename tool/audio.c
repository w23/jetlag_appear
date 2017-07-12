#include "audio.h"
#include "syntmash.h"
#include "timeline.h"
#include <string.h>

#define COUNTOF(c) (sizeof(c)/sizeof(*(c)))
#define MACHINE_STACK 128

static const SymaOp test_program[] = {
	{SYMA_OP_PUSH_STATE, 0, 0},
	{SYMA_OP_PUSH_IN, 1, 0},
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

	{SYMA_OP_POP, 0, 0,}, {SYMA_OP_PSINE, 0, 0},
	//{SYMA_OP_PTRI, 0, 0},

	{SYMA_OP_PUSH_IN, 0, 0},
	{SYMA_OP_MUL, 0, 0}
};

static struct {
	int samplerate;
	float state[2];
	unsigned long samples;
	float time;
} g;

void audioInit(const char *synth_src, int samplerate) {
	(void)synth_src;
	g.samplerate = samplerate;
	g.samples = 0;
	g.time = 0;
	memset(g.state, 0, sizeof(g.state));
}

void audioSynthesize(float *samples, int num_samples) {
	float input[1] = { 440.f / g.samplerate };
	float stack[16];

	SymaRunContext ctx;
	ctx.program = test_program;
	ctx.program_size = COUNTOF(test_program);
	ctx.stack_size = COUNTOF(stack);
	ctx.stack = stack;
	ctx.input_size = COUNTOF(input);
	ctx.input = input;
	ctx.state = g.state;
	ctx.state_size = COUNTOF(g.state);

	for (int i = 0; i < num_samples; ++i, ++g.samples) {
#if 1
		(void)ctx;
		samples[i] = 0;
#else
		const Timeline::Sample sample = timeline.sample(samples_ / (float)samplerate_);
		ctx.input = sample.frame.signal + SCORE_ENVELOPES * MAX_POINT_VALUES;
		ctx.input_size = SAMPLE_SIGNALS - SCORE_ENVELOPES * MAX_POINT_VALUES;
		if (symaRun(&ctx) > 0) {
//			printf("%f %f S ", state_[0], state_[1]);
//			printf("%f %f %f %f %f\n", stack[4], stack[3], stack[2], stack[1], stack[0]);
			samples[i] = stack[0];
		} else
			abort();
#endif
	}

	g.time = g.samples / (float)g.samplerate;
}
