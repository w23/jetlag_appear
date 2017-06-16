#include "Automation.h"
#include <math.h>
#include <memory.h>

static struct {
	int samplerate, bpm;
	int samples_per_bar, samples_per_tick;
	Score score;

	struct {
		struct {
			int env[MAX_PATTERN_ENVELOPES];
			int poly_start, poly_count;
		} instrument[MAX_INSTRUMENTS];
	} map;
} g_;

void automationInit(int samplerate, int bpm) {
	memset(&g_, 255, sizeof(g_));
	g_.samplerate = samplerate;
	g_.bpm = bpm;
	g_.samples_per_bar = g_.samplerate * 4 * 60 / g_.bpm;
	g_.samples_per_tick = g_.samples_per_bar / TICKS_PER_BAR;
}

void automationMapEnv(int instrument, int env, int signal) {
	if (instrument < 0 || instrument >= MAX_INSTRUMENTS) return;
	if (env < 0 || env >= MAX_PATTERN_ENVELOPES) return;
	if (signal < 0 || signal >= MAX_SAMPLE_SIGNALS) return;

	g_.map.instrument[instrument].env[env] = signal;
}

void automationMapInstrument(int instrument, int maxpoly, int start_signal) {
	if (instrument < 0 || instrument >= MAX_INSTRUMENTS) return;
	if (start_signal < 0 || start_signal >= MAX_SAMPLE_SIGNALS) return;
	if (maxpoly < 1 || start_signal + maxpoly * 2 >= MAX_SAMPLE_SIGNALS) return;

	g_.map.instrument[instrument].poly_start = start_signal;
	g_.map.instrument[instrument].poly_count = maxpoly;
}

void automationPatternPut(int pattern, int bar) {
	if (pattern < 0 || pattern >= MAX_SCORE_PATTERNS) return;

	int slot = -1;
	for (int i = 0; i < MAX_SCORE_SIZE; ++i) {
		if (slot == -1 && g_.score.pattern_positions[i].pattern == -1)
			slot = i;
		if (g_.score.pattern_positions[i].pattern == pattern && 
				g_.score.pattern_positions[i].bar == bar)
			return;
	}

	if (slot == -1) return;

	g_.score.pattern_positions[slot].bar = bar;
	g_.score.pattern_positions[slot].pattern = pattern;
}

void automationPatternDel(int pattern, int bar) {
	if (pattern < 0 || pattern >= MAX_SCORE_PATTERNS) return;

	for (int i = 0; i < MAX_SCORE_SIZE; ++i) {
		if (g_.score.pattern_positions[i].pattern == pattern && 
			g_.score.pattern_positions[i].bar == bar) {
			g_.score.pattern_positions[i].pattern = -1;
			break;
		}
	}
}

void automationPatternSetInstrument(int pattern, int instrument) {
	if (pattern < 0 || pattern >= MAX_SCORE_PATTERNS) return;

	g_.score.patterns[pattern].instrument = instrument;
}

void automationPatternNoteAdd(int pattern, int number, int start, int end, int velocity) {
	if (pattern < 0 || pattern >= MAX_SCORE_PATTERNS) return;

	Pattern *pat = g_.score.patterns + pattern;
	if (pat->notes_count == MAX_PATTERN_NOTES) return;

	for (int i = 0; i < MAX_PATTERN_NOTES; ++i)
		if (pat->notes[i].number == number && pat->notes[i].start == start)
			return;

	Note *n = pat->notes + (pat->notes_count++);
	n->number = number;
	n->start = start;
	n->end = end;
	n->velocity = velocity;

	if (pat->length_ticks < n->end)
		pat->length_ticks = n->end;
}

void automationPatternNoteChange(int pattern, int number, int start, int new_end, int new_velocity) {
	if (pattern < 0 || pattern >= MAX_SCORE_PATTERNS) return;

	Pattern *pat = g_.score.patterns + pattern;

	for (int i = 0; i < MAX_PATTERN_NOTES; ++i) {
		Note *n = pat->notes + i;
		if (n->number == number && n->start == start) {
			if (new_end != -1) {
				n->end = new_end;
				if (pat->length_ticks < n->end)
					pat->length_ticks = n->end;
			}
			if (new_velocity != -1)
				n->velocity = new_velocity;
			break;
		}
	}
}

void automationPatternNoteDel(int pattern, int number, int start) {
	if (pattern < 0 || pattern >= MAX_SCORE_PATTERNS) return;
	Pattern *pat = g_.score.patterns + pattern;
	for (int i = 0; i < MAX_PATTERN_NOTES; ++i) {
		Note *n = pat->notes + i;
		if (n->number == number && n->start == start) {
			// TODO update length_ticks
			--pat->notes_count;
			memmove(n, n + 1, sizeof(*n) * (pat->notes_count - i));
			break;
		}
	}
}

void automationPatternEnvPointSet(int pattern, int env, int tick, float v) {
	if (pattern < 0 || pattern >= MAX_SCORE_PATTERNS) return;
	if (env < 0 || env >= MAX_PATTERN_ENVELOPES) return;

	Pattern *pat = g_.score.patterns + pattern;
	Envelope *e = pat->envelopes + env;
	for (int i = 0; i < e->count; ++i) {
		if (e->points[i].t == tick) {
			e->points[i].v = v;
			break;
		}
		if (e->points[i].t > tick) {
			if (e->count == MAX_ENVELOPE_POINTS) return;
			memmove(e->points + i + 1, e->points + i, sizeof(Point) * (e->count - i));
			e->points[i].t = tick;
			e->points[i].v = v;
			++e->count;
			break;
		}
	}
}

void automationPatternEnvPointDel(int pattern, int env, int tick) {
	if (pattern < 0 || pattern >= MAX_SCORE_PATTERNS) return;
	if (env < 0 || env >= MAX_PATTERN_ENVELOPES) return;

	Pattern *pat = g_.score.patterns + pattern;
	Envelope *e = pat->envelopes + env;
	for (int i = 0; i < e->count; ++i) {
		if (e->points[i].t == tick) {
			--e->count;
			memmove(e->points + i, e->points + i + 1, sizeof(Point) * (e->count - i));
			break;
		}
	}
}

static void auto_getFrame(sample_t sample, Frame *frame) {
	int bar = sample / g_.samples_per_bar;

	int poly_demux[MAX_INSTRUMENTS] = {0};

	for (int i = 0; i < MAX_SCORE_SIZE; ++i)
		if (g_.score.pattern_positions[i].pattern != -1 &&
				g_.score.pattern_positions[i].bar < bar) {
			const int ticks_since_start =
				(g_.score.pattern_positions[i].bar - bar * g_.samples_per_bar) / g_.samples_per_tick;
			const Pattern *p = g_.score.patterns + g_.score.pattern_positions[i].pattern;
			if (ticks_since_start >= p->length_ticks)
				continue;

			if (p->instrument == -1)
				continue;

			int *poly = poly_demux + p->instrument;

			for (int j = 0; j < p->notes_count; ++j) {
				const Note *n = p->notes + j;
				if (n->start < ticks_since_start && n->end > ticks_since_start) {
					if (*poly >= g_.map.instrument[p->instrument].poly_count)
						break;

					const int nsig = (*poly) * 2 + g_.map.instrument[p->instrument].poly_start;

					if (nsig + 1 < MAX_SAMPLE_SIGNALS) {
						frame->signal[nsig] = powf(2.f, (n->number - 69) / 12.f) * 440.f / g_.samplerate;
						frame->signal[nsig + 1] = n->velocity / 127.f;
					}

					++(*poly);
				}
			} // for notes

			for (int j = 0; j < MAX_PATTERN_ENVELOPES; ++j) {
				const int nsig = g_.map.instrument[p->instrument].env[j];
				if (nsig < 0 || nsig >= MAX_SAMPLE_SIGNALS) continue;

				const Envelope *e = p->envelopes + j;
				if (e->count < 1) continue;
				
				frame->signal[nsig] = e->points[0].v;
				if (e->points[0].t < ticks_since_start) {
					for (int k = 1; k < MAX_ENVELOPE_POINTS; ++k) {
						if(e->points[k].t > ticks_since_start) {
							const float a = e->points[k-1].v;
							const float b = e->points[k].v;
							const float t = (e->points[k].t - e->points[k-1].t)
								/ (float)(ticks_since_start - e->points[k-1].t);

							frame->signal[nsig] = a + (b - a) * t;
						}
					}
				}
			} // for envelopes
		} // if pattern is active
}

void automationGetSamples(sample_t start, sample_t end, Frame* frames) {
	for (; start < end; ++start, ++frames)
		auto_getFrame(start, frames);
}
