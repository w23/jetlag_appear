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

typedef struct {
	int time;
	enum {
		AEVT_NOTE_ON,
		AEVT_NOTE_OFF,
		AEVT_ENV_SET,
		AEVT_ENV_PREDICT_LINEAR,
	} type;
	int lane;
	unsigned index;
	union {
		struct {
			int num;
			float vel;
		} note;
		struct {
			float value;
			int delay;
		} env;
	} param;
} PatternEvent;

#define MAX_PATTERN_EVENTS 128
#define PATTERN_SIGNALS 16

typedef struct {
	int max_poly;
	int num_events;
	PatternEvent events[MAX_PATTERN_EVENTS];
	unsigned last_index;
} Pattern;

/*
int patternNoteAdd(Pattern *p, int time, int num, int length, float vel);
int patternNoteDel(Pattern *p, unsigned index);

int patternEnvSet(Pattern *p, int lane, int time, float value);
int patternEnvClear(Pattern *p, unsigned index);
*/


// 0: SAMPLERATE, BPM
// 1: absolute time (sec)
// 2: absolute samples (44100/sec)

#define SCORE_ROWS 4
#define ROW_MAX_ENTRIES 16

typedef struct {
	int samplerate, bpm;
	int samples_per_bar, samples_per_tick;
	float seconds_per_tick;
	int bar_end;

	Pattern patterns[SCORE_PATTERNS];
	struct {
		int num_entries;
		struct {
			int bar;
			int pattern;
		} entry[ROW_MAX_ENTRIES];
	} rows[SCORE_ROWS];

	unsigned sequence_;
} Automation;

typedef unsigned int sample_t;

typedef struct {
	unsigned auto_sequence;
	sample_t time;

	struct {
		int last_entry;
		int last_pattern_event;
		struct {
			int on;
			float phase;
			float dphase;
		} notes[PATTERN_SIGNALS];
		struct {
			int tick_start, tick_end;
			float start, end;
		} envs[PATTERN_SIGNALS];
	} row[SCORE_ROWS];
} Cursor;

void automationInit(Automation *a, int samplerate, int bpm);

typedef struct {
	float *signal;
	int start, end;
} Frame;

void automationCursorInit(const Automation *a, Cursor *c);
void automationCursorAdvance(const Automation *a, Cursor *c, sample_t s);
void automationCursorSample(const Automation *a, Cursor *c, Frame *f);

#ifdef __cplusplus
}
#endif
