#include "common.h"
#include "../4klang.h"

#include "atto/app.h"
#define AUDIO_IMPLEMENT
#include "aud_io.h"

#include <stdlib.h> // atoi
#include <string.h>

#define MAX_ACTIVE_TOOLS 4

static struct {
	VolatileResource *project;
	struct {
		Tool head;
		Tool *stack[MAX_ACTIVE_TOOLS];
	} tool_root;
} g;

int toolPush(Tool *t) {
	if (g.tool_root.stack[MAX_ACTIVE_TOOLS-1]) {
		MSG("Too many tools in stack, limit %d", MAX_ACTIVE_TOOLS);
		return 0;
	}

	for (int i = 0; i < MAX_ACTIVE_TOOLS; ++i) {
		if (g.tool_root.stack[i] == t) {
			MSG("Tool %p already in use", t);
			return 0;
		}
	}

	for (int i = MAX_ACTIVE_TOOLS - 1; i > 0; --i) {
		g.tool_root.stack[i] = g.tool_root.stack[i-1];
	}

	g.tool_root.stack[0] = t;
	t->activate(t);
	return 1;
}

void toolPop(Tool *t) {
	for (int i = 0; i < MAX_ACTIVE_TOOLS; ++i) {
		if (g.tool_root.stack[i] == t) {
			for (; i < MAX_ACTIVE_TOOLS - 1; ++i) {
				g.tool_root.stack[i] = g.tool_root.stack[i+1];
			}
			if (t->deactivate)
				t->deactivate(t);
			return;
		}
	}

	MSG("Tool %p not found", t);
}

ToolResult toolMasterProcessEvent(struct Tool *tool, const ToolInputEvent *event) {
	(void)tool;
	assert(tool == &g.tool_root.head);

	for (int i = 0; i < MAX_ACTIVE_TOOLS; ++i) {
		Tool *t = g.tool_root.stack[i];
		if (!t)
			break;

		if (!t->processEvent)
			continue;

		switch (t->processEvent(t, event)) {
		case ToolResult_Ignored:
			continue;
		case ToolResult_Consumed:
			return ToolResult_Consumed;
		case ToolResult_Released:
			toolPop(t);
			return ToolResult_Released;
		}
	}

	switch (event->type) {
	case Input_Key:
		if (!event->e.key.down)
			return ToolResult_Ignored;
		switch (event->e.key.code) {
			case AK_Space:
				audioRawTogglePause();
				break;
			case AK_Right:
				audioRawSeek(audioRawGetTimeBar() + 4.);
				break;
			case AK_Left:
				audioRawSeek(audioRawGetTimeBar() - 4.);
				break;
			case AK_Q:
				aAppTerminate(0);
				break;
			case AK_C:
				toolPush(var_tools.camera);
				break;
			default:
				return ToolResult_Ignored;
		}
		return ToolResult_Consumed;

	case Input_Pointer:
	case Input_MidiCtl:
		return ToolResult_Ignored;
	}

	return ToolResult_Ignored;
}

void toolMasterUpdate(struct Tool *t, float dt) {
	(void)t;
	assert(t == &g.tool_root.head);
	for (int i = 0; i < MAX_ACTIVE_TOOLS; ++i) {
		Tool *t = g.tool_root.stack[i];
		if (!t)
			break;

		if (t->update)
			t->update(t, dt);
	}
}

enum {
	ProjCmd_AudioRaw,
	ProjCmd_VarFile,
	ProjCmd_Scene
};

static int parseAudioRaw(const ParserCallbackParams *params) {
	if (!audioRawInit(params->args[0].s, SAMPLE_RATE, params->args[2].value.i, BPM))
		aAppTerminate(-2);

	return 0;
}

static int parseVariablesFile(const ParserCallbackParams *params) {
	varInit(params->args[0].s);
	return 0;
}

static int parseScene(const ParserCallbackParams *params) {
	MSG("TODO add scene at %d:%d from file %s",
			params->args[0].value.time.bar, params->args[0].value.time.tick,
			params->args[1].s);
	return 0;
}

static const ParserLine project_line_table[] = {
	{"audio_raw", ProjCmd_AudioRaw, 4, 4, {PAF_String, PAF_Int, PAF_Int, PAF_Int}, parseAudioRaw},
	{"variables", ProjCmd_VarFile, 1, 1, {PAF_String}, parseVariablesFile},
	{"scene", ProjCmd_Scene, 2, 2, {PAF_Time, PAF_String}, parseScene},
};

static void parseProject() {
	// TODO clear all state

	ParserTokenizer parser;

	memset(&parser, 0, sizeof(parser));
	parser.ctx.line = g.project->bytes;
	parser.ctx.prev_line = 0;
	parser.ctx.line_number = 0;
	parser.line_table = project_line_table;
	parser.line_table_length = COUNTOF(project_line_table);
	parser.userdata = 0;

	for (;;) {
		const int result = parserTokenizeLine(&parser);
		if (result == Tokenize_End)
			break;

		if (result != Tokenize_Parsed) {
			MSG("Parsing error: %d at line %d",
				result, parser.ctx.line_number);
			break;
		}
	}
}

static void paint(ATimeUs ts, float dt) {
	(void)ts;

	resourcesUpdate();

	if (g.project->updated)
		parseProject();

	const float bars = audioRawGetTimeBar();
	varFrame(bars);

	g.tool_root.head.update(&g.tool_root.head, dt);

	videoOutputResize(a_app_state->width, a_app_state->height);
	videoPaint();
}

static void key(ATimeUs ts, AKey key, int down) {
	(void)(ts);
	ToolInputEvent e;
	e.type = Input_Key;
	e.e.key.code = key;
	e.e.key.down = down;
	g.tool_root.head.processEvent(&g.tool_root.head, &e);
}

static void pointer(ATimeUs ts, int dx, int dy, unsigned int buttons_changed_bits) {
	(void)(ts);
	ToolInputEvent e;
	e.type = Input_Pointer;
	e.e.pointer.x = e.e.pointer.y = 0;
	e.e.pointer.dx = dx;
	e.e.pointer.dy = dy;
	e.e.pointer.buttons_xor = buttons_changed_bits;
	g.tool_root.head.processEvent(&g.tool_root.head, &e);
}

static void audioCallback(void *userdata, float *samples, int nsamples) {
	(void)(userdata);
	audioRawWrite(samples, nsamples);
}

static void midiCallback(void *userdata, const unsigned char *data, int bytes) {
	(void)userdata;

	for (; bytes > 2; bytes -= 3, data += 3) {
		//const int channel = data[0] & 0x0f;
		switch(data[0] & 0xf0) {
			case 0x80:
			case 0x90:
//				timelineMidiNote(data[1], data[2], !!(data[0] & 0x10));
				break;
			case 0xb0:
//				timelineMidiCtl(data[1], data[2]);
				break;
			default:
				MSG("%02x %02x %02x", data[0], data[1], data[2]);
		}
	}
}

static void appClose() {
	audioClose();
}

void attoAppInit(struct AAppProctable *ptbl) {
	ptbl->key = key;
	ptbl->pointer = pointer;
	ptbl->paint = paint;
	ptbl->close = appClose;

	const char *project = "intro.proj";
	int width = 1280, height = 720;
	const char* midi_device = "";
	for (int iarg = 1; iarg < a_app_state->argc; ++iarg) {
		const char *argv = a_app_state->argv[iarg];
		if (strcmp(argv, "-w") == 0)
			width = atoi(a_app_state->argv[++iarg]);
		else if (strcmp(argv, "-h") == 0)
			height = atoi(a_app_state->argv[++iarg]);
		else if (strcmp(argv, "-m") == 0)
			midi_device = a_app_state->argv[++iarg];
	}

	g.tool_root.head.processEvent = toolMasterProcessEvent;
	g.tool_root.head.update = toolMasterUpdate;

	resourcesInit();

	if (!(g.project = resourceOpenFile(project))) {
		MSG("Cannot open project file %s", project);
		aAppTerminate(-1);
	}

	videoInit(width, height, "yo28.scene");

	if (1 != audioOpen(SAMPLE_RATE, 2, NULL, audioCallback, midi_device, midiCallback))
		aAppTerminate(-1);
}
