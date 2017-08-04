#include "common.h"

//#include "seqgui.h"
#include "Automation.h"
#define LFM_IMPLEMENT
#include "lfmodel.h"

#include <stdio.h>
#include <float.h>

static struct {
	int samplerate, bpm;
	LFModel *locked_data;
	LFModel *locked_cursor;
	VolatileResource *timeline_source;
} g;

/*
static void serialize(const char *file, const AmData *a);
*/

typedef struct {
	int ctl;
	int signal;
	float min, mul;
	float value;
} MidiCtlMap;

typedef struct {
	int note_signal;
	int gate_signal;
	int velocity_signal;
} MidiVoiceMap;

#define MAX_MIDI_CTL 32

typedef struct {
	AmData data;
	MidiCtlMap midi_ctl[MAX_MIDI_CTL];
	MidiVoiceMap midi_voice[MAX_MIDI_VOICES];
} DataAndMidi;

typedef struct {
	AmCursor cursor;
	float sig_over[AM_MAX_CURSOR_SIGNALS];
	struct {
		int note, on;
	} midi_voices[MAX_MIDI_VOICES];
} CursorAndOverride;

static struct {
	const char *name;
	enum {
		Token_BPM,
		Token_BarTicks,
		Token_Loop,
		Token_Program,
		Token_Time,
		Token_Set,
		Token_Lin,
		Token_PStart,
		Token_PStop,
		Token_PreviewLoop,
		Token_MidiCtl,
		Token_MidiVoice
	} type;
	int args;
#define MAX_TOKEN_ARGS 8
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
	{"program", Token_Program, 1, {ArgType_Int}},
	{"pstart", Token_PStart, 2, {ArgType_Int, ArgType_Int}},
	{"pend", Token_PStop, 2, {ArgType_Int, ArgType_Int}},
	{"bpm", Token_BPM, 1, {ArgType_Int}},
	{"bar_ticks", Token_BarTicks, 1, {ArgType_Int}},
	{"preview_loop", Token_PreviewLoop, 2, {ArgType_Time, ArgType_Time}},
	{"midi_ctl", Token_MidiCtl, 5, {ArgType_Int, ArgType_Int, ArgType_Float, ArgType_Float, ArgType_Float}},
	{"midi_voice", Token_MidiVoice, 3, {ArgType_Int, ArgType_Int, ArgType_Int}},
};

static int deserialize(int first, const char *source, DataAndMidi *data) {
	AmData *a = &data->data;
	for (int i = 0; i < MAX_MIDI_CTL; ++i)
		data->midi_ctl[i].ctl = -1;
	for (int i = 0; i < MAX_MIDI_VOICES; ++i) {
		MidiVoiceMap *v = data->midi_voice + i;
		v->note_signal = v->gate_signal = v->velocity_signal = -1;
	}

	ParserContext parser;
	parser.line = source;
	parser.prev_line = 0;
	parser.line_number = 0;

	int bpm = 120;
	int bar_ticks = 16;

	int program_index = -1;
	int prev_tick = 0;
	int wait_ticks = 0;
	int op_index = 0;

	int max_time = 0;
	int loop_start = 0, loop_end = 0;

	int midi_ctl_index = 0;
	int midi_voice_index = 0;

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
					argv[j].f = (float)strtod(arg, 0);
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
				loop_start = argv[0].ticks;
				loop_end = argv[1].ticks;
				break;

			case Token_Program:
				if (program_index == -1)
					amDataInit(a, g.samplerate, bpm, bar_ticks);

				program_index = argv[0].i;
				if (program_index < 0 || program_index >= AM_MAX_PROGRAMS) {
					MSG("Program index %d is invalid", program_index);
					return 0;
				}
				for (int j = 0; j < AM_MAX_PROGRAM_OPS; ++j)
					a->programs[program_index].ops[j].type = AmOp_Halt;
				prev_tick = wait_ticks = 0;
				op_index = 0;
				break;

			case Token_Time:
				wait_ticks += argv[0].ticks - prev_tick;
				prev_tick = argv[0].ticks;
				if (max_time < argv[0].ticks)
					max_time = argv[0].ticks;
				break;

			case Token_Lin:
			case Token_Set:
			case Token_Loop:
			case Token_PStart:
			case Token_PStop: {
				if (program_index == -1) {
					MSG("set is a program operation");
					return 0;
				}

				if (op_index >= AM_MAX_PROGRAM_OPS) {
					MSG("too many ops for program %d", program_index);
					return 0;
				}

				if (wait_ticks != 0) {
					AmOp *op = a->programs[program_index].ops + op_index;
					op->type = AmOp_Wait;
					op->a.wait.ticks = amArgImmInt(wait_ticks);
					wait_ticks = 0;
					++op_index;
				}

				if (op_index >= AM_MAX_PROGRAM_OPS) {
					MSG("too many ops for program %d", program_index);
					return 0;
				}

				AmOp *op = a->programs[program_index].ops + op_index;

				if (tokens[itok].type == Token_Set) {
					op->type = AmOp_Signal_Set;
					op->a.signal_set.signal = amArgImmInt(argv[0].i);
					op->a.signal_set.value = amArgImmFloat(argv[1].f);
				} else if (tokens[itok].type == Token_Lin) {
					op->type = AmOp_Signal_Linear;
					op->a.signal_linear.signal = amArgImmInt(argv[0].i);
					op->a.signal_linear.value = amArgImmFloat(argv[1].f);
					op->a.signal_linear.ticks = amArgImmInt(argv[2].ticks);
				} else if (tokens[itok].type == Token_Loop) {
					op->type = AmOp_Loop;
					op->a.loop.ticks = amArgImmInt(0);
				} else if (tokens[itok].type == Token_PStart) {
					op->type = AmOp_Program_Start;
					op->a.program.program = amArgImmInt(argv[0].i);
					op->a.program.core = amArgImmInt(argv[1].i);
					// TODO op->a.program.args
				} else if (tokens[itok].type == Token_PStop) {
					op->type = AmOp_Program_Stop;
					op->a.program.program = amArgImmInt(argv[0].i);
					op->a.program.core = amArgImmInt(argv[1].i);
					// TODO op->a.program.args
				}

				++op_index;
				break;
			}

			case Token_MidiCtl:
				if (midi_ctl_index >= MAX_MIDI_CTL) {
					MSG("Max number of midi ctls %d reached", MAX_MIDI_CTL);
					return 0;
				}

				if (argv[1].i < 0 || argv[1].i >= AM_MAX_CURSOR_SIGNALS) {
					MSG("Max signal %d is invalid [%d, %d)",
							argv[1].i, 0, AM_MAX_CURSOR_SIGNALS);
					return 0;
				}

				data->midi_ctl[midi_ctl_index].ctl = argv[0].i;
				data->midi_ctl[midi_ctl_index].signal = argv[1].i;
				data->midi_ctl[midi_ctl_index].min = argv[2].f;
				data->midi_ctl[midi_ctl_index].mul = (argv[3].f - argv[2].f) / 127.f;

				if (first)
					data->midi_ctl[midi_ctl_index].value = argv[4].f;

				midi_ctl_index++;
				break;
			case Token_MidiVoice:
				if (midi_voice_index >= MAX_MIDI_VOICES) {
					MSG("Max number of midi voices %d reached", MAX_MIDI_VOICES);
					return 0;
				}

				if (argv[0].i >= AM_MAX_CURSOR_SIGNALS) {
					MSG("Max signal %d is invalid [%d, %d)",
							argv[0].i, 0, AM_MAX_CURSOR_SIGNALS);
					return 0;
				}

				if (argv[1].i >= AM_MAX_CURSOR_SIGNALS) {
					MSG("Max signal %d is invalid [%d, %d)",
							argv[1].i, 0, AM_MAX_CURSOR_SIGNALS);
					return 0;
				}

				if (argv[2].i >= AM_MAX_CURSOR_SIGNALS) {
					MSG("Max signal %d is invalid [%d, %d)",
							argv[2].i, 0, AM_MAX_CURSOR_SIGNALS);
					return 0;
				}

				data->midi_voice[midi_voice_index].note_signal = argv[0].i;
				data->midi_voice[midi_voice_index].gate_signal = argv[1].i;
				data->midi_voice[midi_voice_index].velocity_signal = argv[2].i;

				midi_voice_index++;
				break;
		} // switch (token type)
	} // for(;;)

	a->sample_start = a->samples_per_tick * loop_start;
	a->sample_end = a->samples_per_tick * (loop_end == 0 ? max_time : loop_end);

	MSG("Timeline updated");
	return 1;
}

void timelineInit(const char *filename, int samplerate) {
	g.samplerate = samplerate;

	DataAndMidi data;
	amDataInit(&data.data, samplerate, 120, 16);
	g.locked_data = lfmCreate(4, sizeof(DataAndMidi), &data, malloc);

	CursorAndOverride cursor;
	amCursorInit(&data.data, &cursor.cursor);
	for (int i = 0; i < AM_MAX_CURSOR_SIGNALS; ++i)
		cursor.sig_over[i] = FLT_MIN;
	g.locked_cursor = lfmCreate(4, sizeof(CursorAndOverride), &cursor, malloc);

	g.timeline_source = resourceOpenFile(filename);
}

void timelineCheckUpdate() {
	if (!g.timeline_source->updated)
		return;

	DataAndMidi data;
	if (!deserialize(0, g.timeline_source->bytes, &data))
		return;

	// FIXME reset all override signals dangling

	LFLock lock;
	lfmModifyLock(g.locked_data, &lock);
	for (;;) {
		data.data.serial = ((const DataAndMidi*)lock.data_src)->data.serial + 1;
		memcpy(lock.data_dst, &data, sizeof(data));
		if (lfmModifyUnlock(g.locked_data, &lock))
			break;
	}
}

static void copyCursorSignals(const CursorAndOverride *cur, float *output, int signals) {
	signals = signals < AM_MAX_CURSOR_SIGNALS ? signals : AM_MAX_CURSOR_SIGNALS;
	for (int i = 0; i < signals; ++i)
		if (cur->sig_over[i] == FLT_MIN)
			output[i] = cur->cursor.signal_values[i];
		else
			output[i] = cur->sig_over[i];
}

void timelineComputeSignalsAndAdvance(float *output, int signals, int count) {
	(void)count;
	for (;;) {
		LFLock data_lock, cursor_lock;
		lfmReadLock(g.locked_data, &data_lock);
		const DataAndMidi *data = data_lock.data_src;

		lfmModifyLock(g.locked_cursor, &cursor_lock);
		memcpy(cursor_lock.data_dst, cursor_lock.data_src, sizeof(CursorAndOverride));
		CursorAndOverride *cursor = cursor_lock.data_dst;

		if (output) {
			for (int i = 0; i < count; ++i) {
				amCursorAdvance(&data->data, &cursor->cursor, 1);
				copyCursorSignals(cursor, output, signals);
				output += signals;
			}
		} else
				amCursorAdvance(&data->data, &cursor->cursor, count);

		lfmReadUnlock(g.locked_data, &data_lock);
		if (0 != lfmModifyUnlock(g.locked_cursor, &cursor_lock))
			break;
	}
}

void timelineGetLatestSignals(float *output, int signals) {
	LFLock cursor_lock;
	lfmReadLock(g.locked_cursor, &cursor_lock);
	copyCursorSignals(cursor_lock.data_src, output, signals);
	lfmReadUnlock(g.locked_cursor, &cursor_lock);
}

void timelineMidiCtl(int ctl, int value) {
	for (;;) {
		LFLock data_lock, cursor_lock;
		const DataAndMidi *data;
		lfmReadLock(g.locked_data, &data_lock);
		data = data_lock.data_src;

		lfmModifyLock(g.locked_cursor, &cursor_lock);
		memcpy(cursor_lock.data_dst, cursor_lock.data_src, sizeof(CursorAndOverride));
		CursorAndOverride *cursor = cursor_lock.data_dst;

		int i;
		for (i = 0; i < MAX_MIDI_CTL; ++i)
			if (data->midi_ctl[i].ctl == ctl) {
				const MidiCtlMap *map = data->midi_ctl + i;
				if (map->signal >= 0) {
					cursor->sig_over[map->signal] = map->min + value * map->mul;
					MSG("F[%d] = %f", map->signal, cursor->sig_over[map->signal]);
				}
				break;
			}
		if (i == MAX_MIDI_CTL)
			MSG("%d %d", ctl, value);

		lfmReadUnlock(g.locked_data, &data_lock);
		if (0 != lfmModifyUnlock(g.locked_cursor, &cursor_lock))
			break;
	}
}

static void doMidiNote(const DataAndMidi *data, CursorAndOverride *c, int index, int vel) {
	const MidiVoiceMap *voice = data->midi_voice + index;
	if (voice->gate_signal >= 0)
		c->sig_over[voice->gate_signal] = c->midi_voices[index].on;

	if (voice->velocity_signal >= 0)
		c->sig_over[voice->velocity_signal] = vel / 127.f;

	if (voice->note_signal >= 0)
		c->sig_over[voice->note_signal] = c->midi_voices[index].note;
}

void timelineMidiNote(int note, int vel, int on) {
	for (;;) {
		LFLock data_lock, cursor_lock;
		const DataAndMidi *data;
		lfmReadLock(g.locked_data, &data_lock);
		data = data_lock.data_src;

		lfmModifyLock(g.locked_cursor, &cursor_lock);
		memcpy(cursor_lock.data_dst, cursor_lock.data_src, sizeof(CursorAndOverride));
		CursorAndOverride *cursor = cursor_lock.data_dst;

		int i = 0;
		for (i = 0; i < MAX_MIDI_VOICES; ++i)
			if (cursor->midi_voices[i].note == note)
				break;

		if (i == MAX_MIDI_VOICES) {
			i = 0;
			cursor->midi_voices[i].on = 0;
			doMidiNote(data, cursor, i, 0);
		}

		cursor->midi_voices[i].on = on;
		cursor->midi_voices[i].note = note;
		doMidiNote(data, cursor, i, vel);

		MSG("note %d vel %d on %d voice %d", note, vel, on, i);

		lfmReadUnlock(g.locked_data, &data_lock);
		if (0 != lfmModifyUnlock(g.locked_cursor, &cursor_lock))
			break;
	}
}

void timelinePaintUI() {
#if 0
	GuiTransform transform = {0, 0, 1};
	guiPaintAmData(a, 0 /*FIXME now*/, transform);
#endif
}

void timelineEdit(const Event *event) {
	LFLock lock;
	lfmModifyLock(g.locked_data, &lock);
	for (;;) {
		memcpy(lock.data_dst, lock.data_src, sizeof(AmData));

		// TODO
		(void)event;

		if (lfmModifyUnlock(g.locked_data, &lock))
			break;
		lfmModifyRetry(g.locked_data, &lock);
	}
}
