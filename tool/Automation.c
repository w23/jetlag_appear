#include "Automation.h"
#include "common.h"
#include <math.h>
#include <memory.h>
#include <stdlib.h>

#define MAX_CORE_OPS_PER_STEP 64

void amDataInit(AmData *a, int samplerate, int bpm, int ticks_per_bar) {
	memset(a, 0, sizeof(*a));

	a->serial = 0;
	a->samplerate = samplerate;
	a->bpm = bpm;
	a->samples_per_bar = a->samplerate * 4 * 60 / a->bpm;
	a->samples_per_tick = a->samples_per_bar / ticks_per_bar;

	for (int i = 0; i < AM_MAX_PROGRAMS; ++i)
		a->programs[i].epilogue = -1;
}

void amCursorInit(const AmData *a, AmCursor *c) {
	memset(c, 0, sizeof(*c));
	c->data_serial = a->serial;

	c->core[0].program = 0;
	for (int i = 1; i < AM_MAX_CURSOR_CORES; ++i) {
		AmCursorCoreState *core = c->core + i;
		core->program = -1;
	}
}

static AmValue arg(AmArgument arg, const AmCursorCoreState *core) {
	switch(arg.type) {
		case AmArg_Immediate:
			return arg.value.imm;
		case AmArg_Reference:
			return core->args[arg.value.ref % AM_MAX_PROGRAM_ARGS];
	}
	ASSERT(!"Invalid type");
	AmValue value;
	value.i = 31337;
	return value;
}

static void cursorSignalCompute(AmCursor *c, int signal_index) {
	ASSERT(signal_index >= 0);
	ASSERT(signal_index < AM_MAX_CURSOR_SIGNALS);

	const AmCursorSignalState *state = c->signal + signal_index;
	switch(state->mode) {
		case AmSignal_Const:
			c->signal_values[signal_index] = state->base;
			break;
		case AmSignal_Linear:
			c->signal_values[signal_index] = state->base + (c->sample - state->start) * state->lcoeff;
			break;
	}
}

static int cursorCoreStep(const AmData *a, AmCursor *c, int core_index) {
	ASSERT(core_index >= 0);
	ASSERT(core_index < AM_MAX_CURSOR_CORES);
	AmCursorCoreState *core = c->core + core_index;
	if (core->program < 0)
		return 1;
	ASSERT(core->program < AM_MAX_PROGRAMS);

	for (int i = 0; i < MAX_CORE_OPS_PER_STEP; ++i, ++core->next_op) {
		if (core->wait > 0) {
			--core->wait;
			return 1;
		}
		core->next_op = core->next_op % AM_MAX_PROGRAM_OPS;

		const AmProgram *program = a->programs + core->program;
		const AmOp *op = program->ops + core->next_op;
		switch (op->type) {
			case AmOp_Halt:
				if (core->finalizing || program->epilogue < 0) {
					core->program = -1;
					return 1;
				}
				core->finalizing = 1;
				core->next_op = program->epilogue - 1;
				break;
			case AmOp_Wait:
				core->wait = arg(op->a.wait.ticks, core).i * a->samples_per_tick;
				break;
			case AmOp_Loop:
				core->next_op = arg(op->a.loop.ticks, core).i - 1;
				break;
			case AmOp_Signal_Set:
			case AmOp_Signal_Linear: {
				const int signal = arg(op->a.signal_set.signal, core).i;
				if (signal < 0 || signal >= AM_MAX_CURSOR_SIGNALS) {
					MSG("Signal %d is out-of-bounds (0, %d)", signal, AM_MAX_CURSOR_SIGNALS);
					return 0;
				}
				cursorSignalCompute(c, signal); // compute possibly outdated signal
				AmCursorSignalState *state = c->signal + signal;
				state->start = c->sample;
				if (op->type == AmOp_Signal_Set) {
					state->mode = AmSignal_Const;
					state->base = arg(op->a.signal_set.value, core).f;
				} else {
					state->mode = AmSignal_Linear;
					state->base = c->signal_values[signal];
					state->lcoeff = (arg(op->a.signal_linear.value, core).f - state->base)
						/ (arg(op->a.signal_linear.ticks, core).i * a->samples_per_tick);
				}
				cursorSignalCompute(c, signal); // update to current expected value
				break;
			} /* case AmOp_Signal_ */

			case AmOp_Program_Start:
			case AmOp_Program_Stop: {
				const int program_index = arg(op->a.program.program, core).i;
				const int core_index = arg(op->a.program.core, core).i;

				if (program_index < 0 || program_index >= AM_MAX_PROGRAMS) {
					MSG("Invalid program index %d", program_index);
					return 0;
				}
				if (core_index < 0 || core_index >= AM_MAX_CURSOR_CORES) {
					MSG("Invalid core index %d", core_index);
					return 0;
				}

				AmCursorCoreState *target_core = c->core + core_index;
				if (op->type == AmOp_Program_Start) {
					target_core->program = program_index;
					target_core->wait = 0;
					target_core->next_op = (target_core != core) ? 0 : -1;
					target_core->start = c->sample;
					target_core->finalizing = 0;
				} else /* Stop */ {
					const int epilogue = a->programs[program_index].epilogue;
					if (epilogue >= 0 ) {
						// TODO should we kill the core?
						if (!target_core->finalizing) {
							target_core->wait = 0;
							target_core->next_op = (target_core != core) ? epilogue : epilogue - 1;
							target_core->finalizing = 1;
						}
					} else {
						target_core->program = -1;
						if (target_core == core)
							return 1;
					}
				}

				for (int j = 0; j < AM_MAX_PROGRAM_ARGS; ++j)
					target_core->args[j] = arg(op->a.program.args[j], core);
				break;
			} /* AmOp_Program_ */
		} /* switch(op->type) */
	} /* for core ops */

	MSG("Core %d reached max ops per step", core_index);
	return 0;
} /* cursorCoreStep() */

static void cursorFastForward(const AmData *a, AmCursor *c, am_sample_t duration) {
	for (am_sample_t i = 0; i < duration; ++i, ++c->sample)
		for (int j = 0; j < AM_MAX_CURSOR_CORES; ++j)
			cursorCoreStep(a, c, j);
}

void amCursorAdvance(const AmData *a, AmCursor *c, am_sample_t duration) {
	int should_reset = 0;

	if (a->serial != c->data_serial) {
		should_reset = 1;
		duration = 0;
		c->sample = 0;
	}

	if (c->sample < a->sample_start || c->sample >= a->sample_end)
	{
		should_reset = 1;
		duration = 0;
		c->sample = 0;
	} else if (c->sample + duration >= a->sample_end) {
		should_reset = 1;
		duration = (c->sample + duration - a->sample_end) % (a->sample_end - a->sample_start);
	}

	if (should_reset) {
		amCursorInit(a, c);
		cursorFastForward(a, c, a->sample_start);
	}

	cursorFastForward(a, c, duration);

	for (int i = 0; i < AM_MAX_CURSOR_SIGNALS; ++i)
		cursorSignalCompute(c, i);
}
