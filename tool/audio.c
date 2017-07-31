#include "common.h"

#include "syntmash.h"
#include "syntasm.h"
#include "lfmodel.h"

#include <string.h>

#define MACHINE_STACK 128
#define MACHINE_STATE (1024 * 1024)
#define MACHINE_PROGSIZE 1024

static struct {
	int samplerate;
	unsigned long samples;
	float time;
	VolatileResource *source;
	LFModel *lockedMachine;
	unsigned last_sequence;
	float state[MACHINE_STATE];
} g;

typedef struct {
	unsigned sequence;
	SymaRunContext ctx;
	SymaOp program[MACHINE_PROGSIZE];
} RuntimeData;

void audioInit(const char *synth_src, int samplerate) {
	g.samplerate = samplerate;
	g.samples = 0;
	g.time = 0;
	g.source = resourceOpenFile(synth_src);
	RuntimeData data;
	memset(&data, 0, sizeof(data));
	data.ctx.samplerate = g.samplerate;
	g.lockedMachine = lfmCreate(3, sizeof(RuntimeData), &data, malloc);
	g.last_sequence = 0;
	memset(g.state, 0, sizeof(g.state));
}

void audioCheckUpdate() {
	if (!g.source->updated)
		return;

	MSG("Compiling DSP firmware");

	SymaOp program[MACHINE_PROGSIZE];
	SymaRunContext context;
	context.program = program;
	context.program_size = COUNTOF(program);

	if (!symasmCompile(&context, g.source->bytes))
		return;

	LFLock lock;
	lfmModifyLock(g.lockedMachine, &lock);
	const RuntimeData *old_runtime = lock.data_src;
	RuntimeData *runtime = lock.data_dst;
	runtime->sequence = old_runtime->sequence + 1;
	
	memcpy(runtime->program, program, context.program_size * sizeof(program[0]));
	memcpy(&runtime->ctx, &context, sizeof(context));
	runtime->ctx.program = runtime->program;
	
	if (!lfmModifyUnlock(g.lockedMachine, &lock))
		abort();
}

void audioSynthesize(float *samples, int num_samples) {
	float input[16] = { 440.f / g.samplerate };
	float stack[MACHINE_STACK];
	SymaRunContext ctx;

	LFLock lock;
	lfmReadLock(g.lockedMachine, &lock);
	const RuntimeData *runtime = lock.data_src;
	memcpy(&ctx, &runtime->ctx, sizeof(ctx));
	if (runtime->sequence != g.last_sequence) {
		memset(g.state, 0, sizeof(g.state));
		g.last_sequence = runtime->sequence;
	}

	ctx.stack_size = COUNTOF(stack);
	ctx.stack = stack;
	ctx.input_size = COUNTOF(input);
	ctx.input = input;
	ctx.state = g.state;
	ctx.state_size = COUNTOF(g.state);
	ctx.samplerate = g.samplerate;
	ctx.rng = g.samples;

	for (int i = 0; i < num_samples; ++i, ++g.samples) {
		timelineComputeSignalsAndAdvance(input, COUNTOF(input), 1);
		samples[i] = (symaRun(&ctx) > 0) ? stack[0] : 0;
	}

	lfmReadUnlock(g.lockedMachine, &lock);

	g.time = g.samples / (float)g.samplerate;
}
