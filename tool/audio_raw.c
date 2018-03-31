#include "common.h"
#include <stdio.h>
#include <stdlib.h>

static struct {
	const float *buffer;
	unsigned int samples, sample;
	int channels;
	int samplerate;
	int bpm;
	int paused;
	int muted;
} state;

int audioRawInit(const char *filename, int samplerate, int channels, int bpm) {
	FILE* f = fopen(filename, "rb");
	if (!f) {
		MSG("Cannot open %s", filename);
		return 0;
	}

	fseek(f, 0L, SEEK_END);
	const size_t size = ftell(f);
	state.samples = size / sizeof(float) / channels;
	fseek(f, 0L, SEEK_SET);

	float *buffer = malloc(size);
	if (state.samples != fread(buffer, sizeof(float) * channels, state.samples, f)) {
		MSG("Error reading %s", filename);
		free(buffer);
		return 0;
	}
	state.buffer = buffer;
	fclose(f);

	state.channels = channels;
	state.samplerate = samplerate;
	state.bpm = bpm;

	state.sample = 0;
	state.paused = 0;

	return 1;
}

void audioRawWrite(float *samples, int num_samples) {
	memset(samples, 0, num_samples * sizeof(float) * state.channels);

	if (state.paused || !state.samples)
		return;

	for(int i = 0; i < num_samples; ++i) {
		state.sample %= state.samples;
		if (state.buffer)
			for (int j = 0; j < state.channels; ++j)
				samples[i*state.channels + j] = state.buffer[state.channels * state.sample + j] * (state.muted ^ 1);
		++state.sample;
	}
}

int audioRawTogglePause() {
	return state.paused ^= 1;
}

int audioRawToggleMute() {
	return state.muted ^= 1;
}

void audioRawSeek(float bar) {
	if (!state.bpm)
		return;

	state.sample = (unsigned int)(bar * state.samplerate * 60 / state.bpm);
}

float audioRawGetTimeBar() {
	if (!state.bpm)
		return 0;

	return (float)(state.sample * state.bpm) / (state.samplerate * 60);
}

float audioRawGetTimeBar2(float dt) {
	if (!state.bpm)
		return 0;

	state.sample += state.paused * dt * state.samplerate;
	state.sample %= state.samples;

	return (float)(state.sample * state.bpm) / (state.samplerate * 60);
}
