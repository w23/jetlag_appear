#pragma once

#define SCORE_PATTERNS 8
#define SCORE_LENGTH 8
#define PATTERN_BARS 4
#define TICKS_PER_BAR 8
#define PATTERN_TICKS (PATTERN_BARS * TICKS_PER_BAR)
#define SCORE_TICKS (PATTERN_TICKS * SCORE_LENGTH)

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

typedef struct PatternOp {
	enum {
		APOP_HALT,
		APOP_WAIT,
		APOP_LOOP,
		APOP_ENV_SET,
		APOP_ENV_LINEAR,
	} type;
	int lane;
	float value;
	int ticks;
} PatternOp;

#define MAX_PATTERN_OPS 128
#define MAX_PATTERN_ENVS 16

typedef struct {
	PatternOp ops[MAX_PATTERN_OPS];
} Pattern;

typedef struct {
	enum {
		SCOP_HALT,
		SCOP_WAIT,
		SCOP_PATTERN_START,
		SCOP_PATTERN_STOP,
	} type;
	int ticks;
	int row;
	int pattern;
} ScoreOp;

#define MAX_SCORE_OPS 64
#define MAX_SCORE_ROWS 4
#define MAX_SCORE_PATTERNS 16

typedef struct {
	int samplerate, bpm;
	int samples_per_bar, samples_per_tick;
	float seconds_per_tick;

	int version;

	int pattern_envs;
	Pattern patterns[MAX_SCORE_PATTERNS];
	ScoreOp sops[MAX_SCORE_OPS];
} Automation;

typedef unsigned int sample_t;

typedef struct {
	enum {
		EM_CONST,
		EM_LINEAR,
	} mode;
	sample_t start;
	float base, lcoeff;
	float value;
} EnvState;

typedef struct {
	int pattern;
	int wait;
	int pos;
	sample_t start;
	EnvState envs[MAX_PATTERN_ENVS];
} RowState;

typedef struct {
	int data_version;

	sample_t sample;

	int scop_wait;
	int scop_pos;

	RowState row[MAX_SCORE_ROWS];
} AutomCursor;

void automationInit(Automation *a, int samplerate, int bpm);

typedef struct {
	float *signal;
	int start, end;
} Frame;

void automationCursorInit(const Automation *a, AutomCursor *c);
void automationCursorCompute(const Automation *a, AutomCursor *c, const Frame *f);
void automationCursorComputeAndAdvance(const Automation *a, AutomCursor *c, sample_t delta, const Frame *f);

#ifdef __cplusplus
} /* extern "C" */
#endif
