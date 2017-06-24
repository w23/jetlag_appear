#pragma once

#define PATTERN_ENVELOPES 4
#define SCORE_PATTERNS 8
#define SCORE_LENGTH 8
#define PATTERN_BARS 8
#define TICKS_PER_BAR 16
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

#define SCORE_ENVELOPES 4

typedef struct {
	int samplerate, bpm;
	int samples_per_bar, samples_per_tick;

	Envelope env[SCORE_ENVELOPES];
} Automation;

// 0: SAMPLERATE, BPM
// 1: absolute time (sec)
// 2: absolute samples (44100/sec)

void automationInit(Automation *a, int samplerate, int bpm);

Envelope *automationGetEnvelope(Automation *a, int index);

void envGetValues(const Envelope *e, float time, Point *pout);
void envKeypointCreate(Envelope *e, const Point *pin); 
int envKeypointGet(Envelope *e, int time, Point **pout);
void envKeypointDelete(Envelope *e, Point *p);

#define SAMPLE_SIGNALS 32

typedef struct {
	float signal[SAMPLE_SIGNALS];
} Frame;

typedef unsigned int sample_t;

void automationGetSamples(const Automation *a, sample_t start, sample_t end, Frame* frames);

#ifdef __cplusplus
}
#endif
