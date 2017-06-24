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
			a->env[i].points[j].time = -1;
		}
	}
}

Envelope *automationGetEnvelope(Automation *a, int index) {
	if (index < 0 || index > SCORE_ENVELOPES) return NULL;
	return a->env + index;
}

void envGetValues(const Envelope *e, float time, Point *pout) {
	int lt = -1, gt = -1;

	for (int j = 0; j < MAX_ENVELOPE_POINTS; ++j) {
		const Point *p = e->points + j;
		if (p->time < 0)
			break;

		if (p->time <= time)
			lt = j;

		if (p->time > time) {
			gt = j;
			break;
		}
	}

	if (lt < 0) {
		if (gt < 0)
			return;
		if (gt > 0)
			abort();
		memcpy(pout->v, e->points[0].v, sizeof(pout->v));
	}

	if (gt < 0)
		memcpy(pout->v, e->points[lt].v, sizeof(pout->v));

	if (gt == 0)
		abort();

	const Point *pa = e->points + lt, *pb = e->points + gt;
	const float t = (pb->time - time) / (pb->time - pa->time);

	for (int j = 0; j < MAX_POINT_VALUES; ++j)
		pout->v[j] = pb->v[j] + (pa->v[j] - pb->v[j]) * t;
}

void envKeypointCreate(Envelope *e, const Point *pin) {
	for (int i = 0; i < MAX_ENVELOPE_POINTS; ++i) {
		Point *p = e->points + i;
		if (p->time < 0) {
			memcpy(p, pin, sizeof(*pin));
			break;
		}

		if (p->time < pin->time)
			continue;

		if (e->points[MAX_ENVELOPE_POINTS-1].time >= 0)
			return;

		if (p->time == pin->time) {
			if (i < MAX_ENVELOPE_POINTS - 1 && p[1].time == pin->time)
				return;

			++p; ++i;
		}

		memmove(p + 1, p, sizeof(*p) * (MAX_ENVELOPE_POINTS - i - 1));
		memcpy(p, pin, sizeof(*p));
		return;
	}
}

int envKeypointGet(Envelope *e, int time, Point **p) {
	for (int i = 0; i < MAX_ENVELOPE_POINTS; ++i) {
		if (e->points[i].time == time) {
			*p = e->points + i;
			return 1 + (i < MAX_ENVELOPE_POINTS - 1 && e->points[i+1].time == time);
		}
	}
	return 0;
}

void envKeypointDelete(Envelope *e, Point *p) {
	const int index = p - e->points;
	if (index < 0 || index >= MAX_ENVELOPE_POINTS)
		return;

	memmove(p, p + 1, sizeof(Point) * (MAX_ENVELOPE_POINTS - index - 1));
}

static void auto_getFrame(const Automation *a, sample_t sample, Frame *frame) {
	const int global_time = (sample / a->samples_per_tick) % SCORE_TICKS;

	memset(frame, 0, sizeof(*frame));

	for (int i = 0; i < SCORE_ENVELOPES; ++i) {
		const Envelope *e = a->env + i;
		float *signal = frame->signal + i * MAX_POINT_VALUES;

		Point p;
		envGetValues(e, fmod((float)sample / a->samples_per_tick, SCORE_TICKS), &p);

		for (int j = 0; j < MAX_POINT_VALUES; ++j)
			signal[j] = p.v[j];
	}
}

void automationGetSamples(const Automation *a, sample_t start, sample_t end, Frame* frames) {
	for (; start < end; ++start, ++frames)
		auto_getFrame(a, start, frames);
}
