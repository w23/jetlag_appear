#pragma once

#define MAX_ENVELOPE_POINTS 32
#define MAX_PATTERN_NOTES 64
#define MAX_PATTERN_ENVELOPES 8
#define MAX_SCORE_PATTERNS 8
#define MAX_SCORE_SIZE 32

/*
typedef struct {
	int bar;
	int tick;
} Time;
*/

typedef struct {
	int t;
	float v;
} Point;

typedef struct {
	int count;
	Point points[MAX_ENVELOPE_POINTS]; // 256
} Envelope;

typedef struct {
	int number;
	int velocity;
	int start, end; // pattern relative ticks
} Note;

typedef struct {
	int instrument;
	int notes_count, envelopes_count;
	Note notes[MAX_PATTERN_NOTES]; // 1024
	Envelope envelopes[MAX_PATTERN_ENVELOPES]; // ~2048
} Pattern;

typedef struct {
	Pattern patterns[MAX_SCORE_PATTERNS]; // ~24k
	struct {
		int bar, pattern, repeat_count;
	} pattern_positions[MAX_SCORE_SIZE]; // 256
} Score; // < 32k

#define MAX_SAMPLE_SIGNALS 32

typedef struct {
	float signal[MAX_SAMPLE_SIGNALS];
} Frame;

typedef unsigned int sample_t;

// 0: SAMPLERATE, BPM
// 1: absolute time (sec)
// 2: absolute samples (44100/sec)

void automationInit(int samplerate, int bpm);

void automationMapEnv(int instrument, int env, int signal);
void automationMapInstrument(int instrument, int maxpoly, int start_signal);

void automationPatternPut(int pattern, int bar, int repeat_count);
void automationPatternDel(int pattern, int bar, int repeat_count);

void automationPatternNoteAdd(int pattern, int number, int start, int end, int velocity);
void automationPatternNoteChange(int pattern, int number, int start, int end, int velocity);
void automationPatternNoteDel(int pattern, int number, int start);

void automationPatternEnvPointSet(int pattern, int env, int tick, float v);
void automationPatternEnvPointDel(int pattern, int env, int tick);

void automationGetSamples(sample_t start, sample_t end, int frames_count, Frame* frames);
