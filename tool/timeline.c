#include "common.h"

#include "seqgui.h"
#include "Automation.h"
#define LFM_IMPLEMENT
#include "lfmodel.h"

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

static struct {
	int samplerate, bpm;
	LFModel *locked_automation;
	LFModel *locked_cursor;
	VolatileResource *timeline_source;
} g;

/*
static void serialize(const char *file, const Automation *a);
*/

static struct {
	const char *name;
	enum {
		Token_BPM,
		Token_BarTicks,
		Token_Loop,
		Token_Pattern,
		Token_Time,
		Token_Set,
		Token_Lin,
		Token_Score,
		Token_PStart,
		Token_PStop,
		Token_End,
		Token_PreviewLoop,
	} type;
	int args;
#define MAX_TOKEN_ARGS 3
	enum {
		ArgType_Int,
		ArgType_Float,
		ArgType_Time
	} arg_types[MAX_TOKEN_ARGS];
} tokens[] = {
	{"set", Token_Set, 2, {ArgType_Int, ArgType_Float}},
	{"lin", Token_Lin, 3, {ArgType_Int, ArgType_Float, ArgType_Time}},
	{"t", Token_Time, 1, {ArgType_Time}},
	{"loop", Token_Loop, 0, {0}},
	{"pattern", Token_Pattern, 1, {ArgType_Int}},
	{"pstart", Token_PStart, 1, {ArgType_Int}},
	{"pend", Token_PStop, 1, {ArgType_Int}},
	{"score", Token_Score, 0, {0}},
	{"end", Token_End, 0, {0}},
	{"bpm", Token_BPM, 1, {ArgType_Int}},
	{"bar_ticks", Token_BarTicks, 1, {ArgType_Int}},
	{"preview_loop", Token_PreviewLoop, 2, {ArgType_Time, ArgType_Time}},
};

static int deserialize(const char *data, Automation *a) {
	ParserContext parser;
	parser.line = data;
	parser.prev_line = 0;
	parser.line_number = 0;

	int bpm = 120;
	int bar_ticks = 16;

	int pattern_index = -1;
	int prev_tick = 0;
	int wait_ticks = 0;
	int op_index = 0;
	int score = 0;

	for (;;) {
		parseLine(&parser);

		if (parser.status == Status_End)
			break;

		if (parser.status != Status_Parsed) {
			MSG("Parsing error: %d at line %d",
				parser.status, parser.line_number);
			return 0;
		}

		if (parser.tokens < 1)
			continue;

		union {
			int i;
			float f;
			int ticks;
		} argv[MAX_TOKEN_ARGS];
		
		unsigned long itok;
		for (itok = 0; itok < COUNTOF(tokens); ++itok)
			if (strcmp(tokens[itok].name, parser.token[0].str) == 0)
				break;

		if (itok >= COUNTOF(tokens)) {
			MSG("Invalid token '%s'", parser.token[0].str);
			return 0;
		}

		const int args = parser.tokens - 1;
		if (args != tokens[itok].args) {
			MSG("Parsing error: incorrect number of args on line %d", parser.line_number);
			return 0;
		}

		for (int j = 0; j < tokens[itok].args; ++j) {
			const char *arg = parser.token[1+j].str;
			switch(tokens[itok].arg_types[j]) {
				case ArgType_Int:
					argv[j].i = strtol(arg, 0, 10);
					break;
				case ArgType_Float:
					argv[j].f = strtod(arg, 0);
					break;
				case ArgType_Time: {
					int bar, tick;
					if (2 != sscanf(arg, "%d:%d", &bar, &tick)) {
						MSG("Cannot extract bar:tick from '%s'", arg);
						return 0;
					}
					argv[j].ticks = tick + bar * bar_ticks;
					break;
				}
			}
		}

		switch(tokens[itok].type) {
			case Token_BPM:
				bpm = argv[0].i;
				break;

			case Token_BarTicks:
				bar_ticks = argv[0].i;
				break;

			case Token_PreviewLoop:
				// TODO
				break;

			case Token_Pattern:
				if (score) {
					MSG("Already in score");
					return 0;
				}
				if (pattern_index == -1)
					automationInit(a, g.samplerate, bpm, bar_ticks);

				pattern_index = argv[0].i;
				if (pattern_index < 0 || pattern_index >= MAX_SCORE_PATTERNS) {
					MSG("Pattern index %d is invalid", pattern_index);
					return 0;
				}
				for (int j = 0; j < MAX_PATTERN_OPS; ++j)
					a->patterns[pattern_index].ops[j].type = APOP_HALT;
				prev_tick = wait_ticks = 0;
				op_index = 0;
				break;

			case Token_Time:
				wait_ticks += argv[0].ticks - prev_tick;
				prev_tick = argv[0].ticks;
				break;

			case Token_Lin:
			case Token_Set:
			case Token_Loop:
				if (pattern_index == -1) {
					MSG("set is a pattern operation");
					return 0;
				}

				if (op_index >= MAX_PATTERN_OPS) {
					MSG("too many ops for pattern %d", pattern_index);
					return 0;
				}

				if (wait_ticks != 0) {
					PatternOp *op = a->patterns[pattern_index].ops + op_index;
					op->type = APOP_WAIT;
					op->ticks = wait_ticks;
					wait_ticks = 0;
					++op_index;
				}

				if (op_index >= MAX_PATTERN_OPS) {
					MSG("too many ops for pattern %d", pattern_index);
					return 0;
				}

				PatternOp *op = a->patterns[pattern_index].ops + op_index;

				if (tokens[itok].type == Token_Set) {
					op->type = APOP_ENV_SET;
					op->lane = argv[0].i;
					op->value = argv[1].f;
				} else if (tokens[itok].type == Token_Lin) {
					op->type = APOP_ENV_LINEAR;
					op->lane = argv[0].i;
					op->value = argv[1].f;
					op->ticks = argv[2].ticks;
				} else if (tokens[itok].type == Token_Loop) {
					op->type = APOP_LOOP;
				}

				++op_index;
				break;

			case Token_Score:
				score = 1;
				pattern_index = -1;
				prev_tick = wait_ticks = 0;
				op_index = 0;
				break;

			case Token_PStart:
			case Token_PStop:
			case Token_End: {
				if (!score) {
					MSG("score scope expected");
					return 0;
				}

				if (op_index >= MAX_SCORE_OPS) {
					MSG("too many ops for score");
					return 0;
				}

				if (wait_ticks != 0) {
					ScoreOp *op = a->sops + op_index;
					op->type = SCOP_WAIT;
					op->ticks = wait_ticks;
					wait_ticks = 0;
					++op_index;
				}

				if (op_index >= MAX_SCORE_OPS) {
					MSG("too many ops for score");
					return 0;
				}

				ScoreOp *op = a->sops + op_index;
				if (tokens[itok].type == Token_PStart) {
					op->type = SCOP_PATTERN_START;
					op->row = op->pattern = argv[0].i;
				} else if (tokens[itok].type == Token_PStop) {
					op->type = SCOP_PATTERN_STOP;
					op->row = op->pattern = argv[0].i;
				} else if (tokens[itok].type == Token_End) {
					op->type = SCOP_HALT;
				}

				++op_index;
				break;
			}
		}
	} // for(;;)

	MSG("Timeline updated");
	return 1;
}

void timelineInit(const char *filename, int samplerate) {
	g.samplerate = samplerate;
	g.locked_automation = lfmCreate(4, sizeof(Automation), NULL, malloc);
	g.locked_cursor = lfmCreate(4, sizeof(AutomCursor), NULL, malloc);
	g.timeline_source = resourceOpenFile(filename);

	Automation a;
	automationInit(&a, samplerate, 120, 16);
	LFLock lock;
	lfmModifyLock(g.locked_automation, &lock);
	memcpy(lock.data_dst, &a, sizeof(a));
	ASSERT(lfmModifyUnlock(g.locked_automation, &lock));

	AutomCursor c;
	automationCursorInit(&a, &c);
	lfmModifyLock(g.locked_cursor, &lock);
	memcpy(lock.data_dst, &c, sizeof(c));
	ASSERT(lfmModifyUnlock(g.locked_cursor, &lock));
}

void timelineCheckUpdate() {
	if (!g.timeline_source->updated)
		return;

	Automation a;
	if (!deserialize(g.timeline_source->bytes, &a))
		return;

	LFLock lock;
	lfmModifyLock(g.locked_automation, &lock);
	for (;;) {
		a.version = ((const Automation*)lock.data_src)->version + 1;
		memcpy(lock.data_dst, &a, sizeof(a));
		if (lfmModifyUnlock(g.locked_automation, &lock))
			break;
	}
}

void timelineGetSignals(float *output, int signals, int count, int advance) {
	LFLock automation_lock;
	lfmReadLock(g.locked_automation, &automation_lock);
	{
		LFLock cursor_lock;
		Frame f;
		f.signal = output;
		f.start = 0;
		f.end = signals;
		if (advance) {
			lfmModifyLock(g.locked_cursor, &cursor_lock);
			memcpy(cursor_lock.data_dst, cursor_lock.data_src, sizeof(AutomCursor));
			automationCursorComputeAndAdvance(automation_lock.data_src, cursor_lock.data_dst, count, &f);
			ASSERT(lfmModifyUnlock(g.locked_cursor, &cursor_lock));
		} else {
			AutomCursor c;
			lfmReadLock(g.locked_cursor, &cursor_lock);
			memcpy(&c, cursor_lock.data_src, sizeof(c));
			ASSERT(lfmReadUnlock(g.locked_cursor, &cursor_lock));
			automationCursorCompute(automation_lock.data_src, &c, &f);
		}
	}
	lfmReadUnlock(g.locked_automation, &automation_lock);
}

void timelinePaintUI() {
#if 0
	GuiTransform transform = {0, 0, 1};
	guiPaintAutomation(a, 0 /*FIXME now*/, transform);
#endif
}

void timelineEdit(const Event *event) {
	LFLock lock;
	lfmModifyLock(g.locked_automation, &lock);
	for (;;) {
		memcpy(lock.data_dst, lock.data_src, sizeof(Automation));

		// TODO
		(void)event;

		if (lfmModifyUnlock(g.locked_automation, &lock))
			break;
		lfmModifyRetry(g.locked_automation, &lock);
	}
}
