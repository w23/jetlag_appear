#pragma once

#define PATTERN_ENVELOPES 4
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

#define MAX_POINT_VALUES 4

typedef struct {
	int time;
	float v[MAX_POINT_VALUES];
} Point;

#define MAX_ENVELOPE_POINTS 64

typedef struct {
	Point points[MAX_ENVELOPE_POINTS];
} Envelope;

void envGetValues(const Envelope *e, float time, Point *pout);
void envKeypointCreate(Envelope *e, const Point *pin); 
int envKeypointGet(Envelope *e, int time, Point **pout);
void envKeypointDelete(Envelope *e, Point *p);

typedef struct {
	int event : 1;
	int off : 1;
	int num : 8;
} Note;

typedef struct {
	Note notes[PATTERN_TICKS];
} Pattern;

typedef struct {
	float notenum;
	float gate;
	float time_since_last_on;
//	float adsr;
} NoteSignal;

void patGetSignal(const Pattern *p, float t, NoteSignal *sig_out);

// 0: SAMPLERATE, BPM
// 1: absolute time (sec)
// 2: absolute samples (44100/sec)

#define SCORE_ENVELOPES 4

typedef struct {
	int samplerate, bpm;
	int samples_per_bar, samples_per_tick;
	float seconds_per_tick;

	Envelope env[SCORE_ENVELOPES];
	Pattern patterns[SCORE_PATTERNS];
} Automation;

void automationInit(Automation *a, int samplerate, int bpm);

Envelope *automationGetEnvelope(Automation *a, int index);

#define SAMPLE_SIGNALS 32

typedef struct {
	float signal[SAMPLE_SIGNALS];
} Frame;

typedef unsigned int sample_t;

void automationGetSamples(const Automation *a, sample_t start, sample_t end, Frame* frames);

#ifdef __cplusplus
}
#endif
