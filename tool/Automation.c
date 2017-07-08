#include "Automation.h"
#include <math.h>
#include <memory.h>
#include <stdlib.h>

void automationInit(Automation *a, int samplerate, int bpm) {
	a->sequence_ = 0;
	a->samplerate = samplerate;
	a->bpm = bpm;
	a->samples_per_bar = a->samplerate * 4 * 60 / a->bpm;
	a->samples_per_tick = a->samples_per_bar / TICKS_PER_BAR;
	a->seconds_per_tick = (float)a->samples_per_tick / a->samplerate;
	a->bar_end = 1;

	for (int i = 0; i < SCORE_PATTERNS; ++i) {
		Pattern *p = a->patterns + i;
		p->max_poly = 0;
		p->num_events = 0;
		p->last_index = 0;
	}

	for (int i = 0; i < SCORE_ROWS; ++i) {
		a->rows[i].num_entries = 0;
	}
}

void automationCursorInit(const Automation *a, Cursor *c) {
	memset(c, 0, sizeof(*c));
	c->auto_sequence = a->sequence_;

	automationCursorAdvance(a, c, 0);
}

void automationCursorAdvance(const Automation *a, Cursor *c, sample_t s) {
	if (a->sequence_ != c->auto_sequence) {
		s += c->time;
		automationCursorInit(a, c);
	}

	int bar = s / a->samples_per_bar;
	if (bar >= a->bar_end) {
		bar = a->bar_end - 1;
		s = a->bar_end * a->samples_per_bar;
	}

	for (int i = 0; i < SCORE_ROWS; ++i) {
		for (int j = c->row[i].last_entry; j < a->rows[i].num_entries; ++j) {
			if (a->rows[i].entry[j].bar > 
			const Pattern *p = a->patterns
		}
	}
}

void automationCursorSample(const Automation *a, Cursor *c, Frame *f) {
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

void patGetSignal(const Pattern *p, float t, NoteSignal *sig_out) {
	const int tick = (int)floor(t) % PATTERN_TICKS;
	int last_note_on = -1;
	sig_out->gate = 0;
	sig_out->notenum = 0;
	sig_out->time_since_last_on = 0;
	for (int i = tick + 1; i <= PATTERN_TICKS + tick; ++i) {
		const Note *n = p->notes + i % PATTERN_TICKS;
		if (n->event) {
			if (!n->off) {
				last_note_on = i;
				sig_out->gate = 1;
				sig_out->notenum = n->num;
			} else
				sig_out->gate = 0;
		}
	}

	sig_out->notenum = 440.f * powf(2.f, (sig_out->notenum - 69.f) / 12.f);
	sig_out->time_since_last_on = last_note_on - PATTERN_TICKS - tick + fmodf(t, 1.f);
}

static void writeSignal(const float *src, int count, int offset, Frame *frame) {
	for (int i = 0; i < count && i + offset < frame->end; ++i) {
		if (i + offset >= frame->start)
			frame->signal[offset + i] = src[i];
	}
}

static void auto_getFrame(const Automation *a, sample_t sample, Frame *frame) {
	for (int i = 0; i < SCORE_ENVELOPES; ++i) {
		const Envelope *e = a->env + i;

		Point p;
		envGetValues(e, fmodf((float)sample / a->samples_per_tick, SCORE_TICKS), &p);
		writeSignal(p.v, MAX_POINT_VALUES, i * MAX_POINT_VALUES, frame);
	}

	const float pat_tick_time = fmodf((float)sample / a->samples_per_tick, PATTERN_TICKS);

	for (int i = 0; i < SCORE_PATTERNS; ++i) {
		const Pattern *p = a->patterns + i;
#define PATTERN_SIGNALS 3
		const int sig_index = SCORE_ENVELOPES * MAX_POINT_VALUES + i * PATTERN_SIGNALS;
		
		NoteSignal sig;
		patGetSignal(p, pat_tick_time, &sig);
		const float real_signal[PATTERN_SIGNALS] = {
				sig.gate,
				sig.notenum / a->samplerate,
				sig.time_since_last_on * a->seconds_per_tick
			};
		writeSignal(real_signal, PATTERN_SIGNALS, sig_index, frame);
	}
}

void automationGetSamples(const Automation *a, sample_t start, sample_t end, Frame* frames) {
	for (; start < end; ++start, ++frames)
		auto_getFrame(a, start, frames);
}
