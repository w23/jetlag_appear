#include "Automation.h"
#include <math.h>
#include <memory.h>
#include <stdlib.h>

void automationInit(Automation *a, int samplerate, int bpm) {
	a->samplerate = samplerate;
	a->bpm = bpm;
	a->samples_per_bar = a->samplerate * 4 * 60 / a->bpm;
	a->samples_per_tick = a->samples_per_bar / TICKS_PER_BAR;

	for (int i = 0; i < SCORE_ENVELOPES; ++i) {
		for (int j = 0; j < MAX_ENVELOPE_POINTS; ++j) {
			a->env[i].points[j].tick = -1;
		}
	}
}

void automationEnvPointSet(Automation *a, int env, int tick, float v[MAX_POINT_VALUES]) {
	if (env < 0 || env >= SCORE_ENVELOPES) return;
	if (tick < 0 || tick >= SCORE_TICKS) return;

	Envelope *e = a->env + env;
	for (int i = 0; i < MAX_ENVELOPE_POINTS; ++i) {
		Point *p = e->points + i;
		if (p->tick < 0) {
			p->tick = tick;
			memcpy(p->v, v, sizeof(p->v));
			break;
		}
		if (e->points[i].tick == tick) {
			memcpy(p->v, v, sizeof(p->v));
			break;
		}
		if (e->points[i].tick > tick) {
			if (e->points[MAX_ENVELOPE_POINTS-1].tick >= 0)
				return;

			memmove(e->points + i + 1, e->points + i, sizeof(Point) * (MAX_ENVELOPE_POINTS - i - 1));
			e->points[i].tick = tick;
			memcpy(p->v, v, sizeof(p->v));
			break;
		}
	}

	for (int i = 0; i < MAX_ENVELOPE_POINTS; ++i) {
		Point *p = e->points + i;
		printf("%d: %d %f %f %f\n", i, p->tick, p->v[0], p->v[1], p->v[2]);
	}
}

void automationEnvPointDel(Automation *a, int env, int tick) {
	if (env < 0 || env >= SCORE_ENVELOPES) return;
	if (tick < 0 || tick >= SCORE_TICKS) return;

	Envelope *e = a->env + env;
	for (int i = 0; i < MAX_ENVELOPE_POINTS; ++i) {
		Point *p = e->points + i;
		if (e->points[i].tick == tick) {
			memmove(e->points + i, e->points + i + 1, sizeof(Point) * (MAX_ENVELOPE_POINTS - i - 1));
			break;
		}
	}
}

static void auto_getFrame(const Automation *a, sample_t sample, Frame *frame) {
	const int global_tick = (sample / a->samples_per_tick) % SCORE_TICKS;

	memset(frame, 0, sizeof(*frame));

	for (int i = 0; i < SCORE_ENVELOPES; ++i) {
		const Envelope *e = a->env + i;
		float *signal = frame->signal + i * MAX_POINT_VALUES;

		int lt = -1, gt = -1;

		for (int j = 0; j < MAX_ENVELOPE_POINTS; ++j) {
			const Point *p = e->points + j;
			if (p->tick < 0)
				break;

			if (p->tick <= global_tick)
				lt = j;

			if (p->tick > global_tick) {
				gt = j;
				break;
			}
		}

		if (lt < 0) {
			if (gt < 0)
				continue;
			if (gt > 0)
				abort();
			memcpy(signal, e->points[0].v, sizeof(float) * MAX_POINT_VALUES);
		}

		if (gt < 0) {
			memcpy(signal, e->points[lt].v, sizeof(float) * MAX_POINT_VALUES);
		}

		if (gt == 0)
			abort();

		const Point *pa = e->points + lt, *pb = e->points + gt;
		const float t = (float)(pb->tick * a->samples_per_tick - sample) / ((pb->tick - pa->tick) * a->samples_per_tick);

		for (int j = 0; j < MAX_POINT_VALUES; ++j)
			signal[j] = pa->v[j] + (pb->v[j] - pa->v[j]) * t;
	}
}

void automationGetSamples(const Automation *a, sample_t start, sample_t end, Frame* frames) {
	for (; start < end; ++start, ++frames)
		auto_getFrame(a, start, frames);
}
