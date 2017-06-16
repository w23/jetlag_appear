#include "Audio.h"
#include <math.h>

#define COUNTOF(c) (sizeof(c)/sizeof(*(c)))

#define MACHINE_STACK 128

namespace {
const op_t test_program[] = {
	{op_t::OP_PUSH_STATE, {0, 0}},
	{op_t::OP_PUSH_IN, {0, 0}},
	{op_t::OP_ADD, {0, 0}},
	{op_t::OP_FRACT, {0, 0}},
	{op_t::OP_DUP, {0, 0}},
	{op_t::OP_POP_STATE, {0, 0}},
	//{op_t::OP_PUSH, {0, .5f}},

	{op_t::OP_PUSH_STATE, {1, 0}},
	{op_t::OP_PUSH, {0, .5f / 44100.f}},
	{op_t::OP_ADD, {0, 0}},
	{op_t::OP_FRACT, {0, 0}},
	{op_t::OP_DUP, {0, 0}},
	{op_t::OP_POP_STATE, {1, 0}},

	{op_t::OP_PSINE, {0, 0}},
	{op_t::OP_PUSH, {0, 1.f}},
	{op_t::OP_ADD, {0, 0}},
	{op_t::OP_PUSH, {0, .5f}},
	{op_t::OP_MUL, {0, 0}},

	{op_t::OP_PTRI, {0, 0}}
	//{op_t::OP_PSINE, {0, 0}}
};
}

float generateSample(const float *inputs, machine_t *m) {
	float stack[MACHINE_STACK];
	int sp = -1;
	for (int i = 0; i < m->program_size; ++i) {
		const op_t *op = m->program + i;
		switch (op->op) {
		case op_t::OP_PUSH: stack[++sp] = op->arg.f; break;
		case op_t::OP_PUSH_STATE: stack[++sp] = m->state[op->arg.i]; break;
		case op_t::OP_POP_STATE: m->state[op->arg.i] = stack[sp--]; break;
		case op_t::OP_PUSH_IN: stack[++sp] = inputs[op->arg.i]; break;
		case op_t::OP_DUP: { float v = stack[sp]; stack[++sp] = v; } break;
		case op_t::OP_ADD: { float v = stack[sp]; stack[--sp] += v; } break;
		case op_t::OP_MUL: { float v = stack[sp]; stack[--sp] *= v; } break;
		case op_t::OP_PSINE: stack[sp] = sinf(3.1415927f * 2.f * stack[sp]); break;
		case op_t::OP_PTRI: {
			float v = stack[sp-1], p = stack[sp];
			stack[--sp] = ((v < p) ? 1.f - v / p : (v - p) / (1.f - p)) * 2.f - 1.f;
			break;
		}
		case op_t::OP_FRACT: stack[sp] = fmodf(stack[sp], 1.f); break;
		}
	}

	return stack[0];
}

Audio::Audio(int samplerate, const char *audio_file)
	: samplerate_(samplerate)
{
	(void)audio_file;

	machine_.state = state_;
	machine_.n_state = 1;
	machine_.n_inputs = 1;
	machine_.program = test_program;
	machine_.program_size = COUNTOF(test_program);
}

void Audio::synthesize(float *samples, int num_samples /*Timeline*/) {
	float input[1] = { 440.f / samplerate_ };
	for (int i = 0; i < num_samples; ++i) {
		samples[i] = generateSample(input, &machine_);
	}
}
