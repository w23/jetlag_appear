#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/*
typedef struct {
	int bar;
	int tick;
} Time;

- seconds
- samples (44100 / sec)
- bar_seconds = bpm / 4 (beats) / 60 (seconds)
- tick_seconds = bar_seconds / 16
- bar_samples = 44100 / bar_seconds
- tick_samples = bar_samples / 16
*/

#define AM_MAX_PROGRAM_ARGS 4
#define AM_MAX_PROGRAM_OPS 128
#define AM_MAX_PROGRAMS 8
#define AM_MAX_CURSOR_CORES 16
#define AM_MAX_CURSOR_SIGNALS 32

typedef enum {
	AmOp_Halt,
	AmOp_Wait,
	AmOp_Loop,
	AmOp_Signal_Set,
	AmOp_Signal_Linear,
	AmOp_Program_Start,
	AmOp_Program_Stop,
} AmOpType;

typedef union {
	float f;
	int i;
} AmValue;

typedef struct {
	enum {
		AmArg_Immediate,
		AmArg_Reference,
	} type;
	union {
		int ref;
		AmValue imm;
	} value;
} AmArgument;

static inline AmArgument amArgImmInt(int i) {
	AmArgument arg;
	arg.type = AmArg_Immediate;
	arg.value.imm.i = i;
	return arg;
}

static inline AmArgument amArgImmFloat(float f) {
	AmArgument arg;
	arg.type = AmArg_Immediate;
	arg.value.imm.f = f;
	return arg;
}

static inline AmArgument amArgRef(int ref) {
	AmArgument arg;
	arg.type = AmArg_Reference;
	arg.value.ref = ref;
	return arg;
}

typedef struct {
	AmOpType type;
	union {
		struct {
			AmArgument ticks;
		} wait;
		struct {
			AmArgument ticks;
		} loop;
		struct {
			AmArgument signal;
			AmArgument value;
		} signal_set;
		struct {
			AmArgument signal;
			AmArgument value;
			AmArgument ticks;
		} signal_linear;
		struct {
			AmArgument program;
			AmArgument core;
			AmArgument args[AM_MAX_PROGRAM_ARGS];
		} program;
	} a;
} AmOp;

typedef struct {
	int epilogue;
	AmOp ops[AM_MAX_PROGRAM_OPS];
} AmProgram;

typedef struct {
	int serial;
	int samplerate, bpm;
	int samples_per_bar, samples_per_tick;

	AmProgram programs[AM_MAX_PROGRAMS];
} AmData;

typedef unsigned am_sample_t;

typedef struct {
	enum {
		AmSignal_Const,
		AmSignal_Linear,
	} mode;
	am_sample_t start;
	float base, lcoeff;
} AmCursorSignalState;

typedef struct {
	AmValue args[AM_MAX_PROGRAM_ARGS];
	int program;
	am_sample_t wait;
	int next_op;
	am_sample_t start;
} AmCursorCoreState;

typedef struct {
	int data_serial;

	am_sample_t sample;

	AmCursorCoreState core[AM_MAX_CURSOR_CORES];
	AmCursorSignalState signal[AM_MAX_CURSOR_SIGNALS];
	float signal_values[AM_MAX_CURSOR_SIGNALS];
} AmCursor;

void amDataInit(AmData *a, int samplerate, int bpm, int ticks_per_bar);

void amCursorInit(const AmData *a, AmCursor *c);
void amCursorAdvance(const AmData *a, AmCursor *c, am_sample_t delta);

#ifdef __cplusplus
} /* extern "C" */
#endif
