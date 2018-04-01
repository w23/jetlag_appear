#include "common.h"
#include <stdio.h>
#include <stdlib.h>

static struct {
	const float *buffer;
	unsigned int samples, sample;
	int channels;
	int samplerate;
	int bpm;
	unsigned int samples_per_bar;

	int paused;
	int muted;

	struct {
		enum {
			LoopState_Off,
			LoopState_StartSet,
			LoopState_Looping
		} state;
		unsigned int a, b;
	} loop;
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
	state.samples_per_bar = state.samplerate * 60 / state.bpm;

	state.sample = 0;
	state.paused = 0;

	return 1;
}

void audioRawWrite(float *samples, int num_samples) {
	memset(samples, 0, num_samples * sizeof(float) * state.channels);

	if (state.paused || !state.samples)
		return;

	const unsigned int start = state.loop.state == LoopState_Looping ? state.loop.a : 0;
	const unsigned int end = state.loop.state == LoopState_Looping ? state.loop.b : state.samples;
	const unsigned int length = end - start;

	if (state.sample < start)
		state.sample = start;

	unsigned int offset = state.sample - start;

	for(int i = 0; i < num_samples; ++i) {
		offset %= length;
		if (state.buffer)
			for (int j = 0; j < state.channels; ++j)
				samples[i*state.channels + j] = state.buffer[state.channels * (start + offset) + j] * (state.muted ^ 1);
		++offset;
	}

	state.sample = start + offset;
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

	if (bar > 0) {
		const unsigned int delta = (unsigned int)(bar * state.samples_per_bar);
		if (state.samples - state.sample > delta)
			state.sample += delta;
	} else if (bar < 0) {
		const unsigned int delta = (unsigned int)(-bar * state.samples_per_bar);
		if (state.sample > delta)
			state.sample -= delta;
		else
			state.sample = 0;
	}

	MSG("seek to bar %f", audioRawGetTimeBar());
}

unsigned int nearestBarSample(int gran) {
	const unsigned int bar = (unsigned int)floorf(audioRawGetTimeBar());
	return (bar - bar % gran) * state.samples_per_bar;
}

void audioRawLoopToggle() {
	if (!state.bpm)
		return;

	switch (state.loop.state) {
	case LoopState_Off:
		state.loop.a = nearestBarSample(4);
		MEMORY_BARRIER();
		state.loop.state = LoopState_StartSet;
		MSG("Loop start: %d", state.loop.a / state.samples_per_bar);
		break;
	case LoopState_StartSet:
		state.loop.b = nearestBarSample(4);
		MEMORY_BARRIER();
		state.loop.state = LoopState_Looping;
		MSG("Loop: %d -> %d", state.loop.a / state.samples_per_bar, state.loop.b / state.samples_per_bar);
		break;
	case LoopState_Looping:
		state.loop.state = LoopState_Off;
		MSG("Loop off");
		break;
	}
}

float audioRawGetTimeBar() {
	if (!state.bpm)
		return 0;

	return (float)(state.sample * state.bpm) / (state.samplerate * 60);
}

float audioRawGetTimeBar2(float dt) {
	if (!state.bpm)
		return 0;

	state.sample += (state.paused ^ 1) * dt * state.samplerate;
	state.sample %= state.samples;

	return (float)state.sample / (float)state.samples_per_bar;
}
