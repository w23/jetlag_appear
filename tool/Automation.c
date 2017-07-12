#include "Automation.h"
#include <math.h>
#include <memory.h>
#include <stdlib.h>
#include <assert.h>

void automationInit(Automation *a, int samplerate, int bpm) {
	memset(a, 0, sizeof(*a));

	a->version = 0;
	a->samplerate = samplerate;
	a->bpm = bpm;
	a->samples_per_bar = a->samplerate * 4 * 60 / a->bpm;
	a->samples_per_tick = a->samples_per_bar / TICKS_PER_BAR;
	a->seconds_per_tick = (float)a->samples_per_tick / a->samplerate;
}

void automationAutomCursorInit(const Automation *a, AutomCursor *c) {
	memset(c, 0, sizeof(*c));
	c->data_version = a->version;
}

static void writeSignal(const float *src, int count, int offset, int index, const Frame *frame) {
	for (int i = 0; i < count && i + offset < frame->end; ++i) {
		if (i + offset >= frame->start)
			frame->signal[offset + i + index * (frame->end - frame->start)] = src[i];
	}
}

static void cursorScopAdvance(const Automation *a, AutomCursor *c, sample_t sample) {
	for (;c->scop_pos < MAX_SCORE_OPS; ++c->scop_pos) {
		const ScoreOp *op = a->sops + c->scop_pos;
		switch (op->type) {
			case SCOP_HALT:
				return;
			case SCOP_WAIT:
				c->scop_wait = op->ticks * a->samples_per_tick;
				return;
			case SCOP_PATTERN_START:
				if (op->row < 0 || op->row >= MAX_SCORE_ROWS) break;
				memset(c->row + op->row, 0, sizeof(c->row[0]));
				c->row[op->row].pattern = op->pattern;
				c->row[op->row].start = sample;
				break;
			case SCOP_PATTERN_STOP:
				if (op->row < 0 || op->row >= MAX_SCORE_ROWS) break;
				memset(c->row + op->row, 0, sizeof(c->row[0]));
				c->row[op->row].pattern = -1;
				break;
		}
	}
}

static void cursorRowAdvance(const Automation *a, AutomCursor *c, int row, sample_t sample) {
	assert(row > 0 && row < MAX_SCORE_ROWS);
	RowState *rs = c->row + row;
	assert(rs->pattern >= 0 && rs->pattern < MAX_SCORE_PATTERNS);
	assert(rs->wait == 0);

	const Pattern *pattern = a->patterns + rs->pattern;

	for (int i = 0; i < MAX_PATTERN_OPS; ++i) {
		const PatternOp *op = pattern->ops + rs->pos;
		switch (op->type) {
			case APOP_HALT:
				return;
			case APOP_WAIT:
				rs->wait = op->ticks * a->samples_per_tick;
				return;
			case APOP_LOOP:
				rs->pos = 0;
				continue;
			case APOP_ENV_SET:
				if (op->lane < 0 || op->lane >= MAX_PATTERN_ENVS)
					break;
				{
					EnvState *es = rs->envs + op->lane;
					es->mode = EM_CONST;
					es->base = es->value = op->value;
				}
				break;
			case APOP_ENV_LINEAR:
				if (op->lane < 0 || op->lane >= MAX_PATTERN_ENVS)
					break;
				{
					EnvState *es = rs->envs + op->lane;
					es->mode = EM_LINEAR;
					es->base = es->value;
					es->start = sample;
					es->lcoeff = (op->value - es->base) / (op->ticks * a->samples_per_tick);
				}
				break;
		}
		++rs->pos;
	}

	assert(!"too many instr");
}

static void cursorComputeRowSignals(const Automation *a, AutomCursor *c, int row, int frame_idx, const Frame *f) {
	float envs[MAX_PATTERN_ENVS];
	for (int k = 0; k < MAX_PATTERN_ENVS; ++k) {
		envs[k] = 0;
		EnvState *es = c->row[row].envs + k;
		switch (es->mode) {
			case EM_CONST:
				es->value = envs[k] = es->base;
				break;
			case EM_LINEAR:
				es->value = envs[k] = es->base + (c->sample - es->start) * es->lcoeff;
		}
	} // for all envs
	if (f)
		writeSignal(envs, MAX_PATTERN_ENVS, row * a->pattern_envs, frame_idx, f);
}

void automationAutomCursorCompute(const Automation *a, AutomCursor *c, const Frame *f) {
	for (int j = 0; j < MAX_SCORE_ROWS; ++j) {
		if (c->row[j].pattern < 0) continue;
		cursorComputeRowSignals(a, c, j, 0, f);
	} // for all rows 
}

void automationAutomCursorComputeAndAdvance(const Automation *a, AutomCursor *c, sample_t duration, const Frame *f) {
	for (sample_t i = 0; i < duration; ++i, ++c->sample) {
		if (c->scop_wait)
			--c->scop_wait;
		else
			cursorScopAdvance(a, c, c->sample);

		for (int j = 0; j < MAX_SCORE_ROWS; ++j) {
			if (c->row[j].pattern < 0) continue;

			if (c->row[j].wait)
				--c->row[j].wait;
			else
				cursorRowAdvance(a, c, j, c->sample);

			cursorComputeRowSignals(a, c, j, i, f);
		} // for all rows 
	} // for samples
}
