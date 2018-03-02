#include "common.h"
#include "../4klang.h"

#include "atto/app.h"
#define AUDIO_IMPLEMENT
#include "aud_io.h"

#include <stdlib.h> // atoi
#include <string.h>

static struct {
	VolatileResource *project;
} g;

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
	MSG("TODO variables file: %s", params->args[0].s);
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
	(void)dt; (void)ts;
	//const float now = 1e-6f * ts;

	resourcesUpdate();

	if (g.project->updated)
		parseProject();

	{
		float signals[32];
		memset(signals, 0, sizeof(signals));
		signals[0] = audioRawGetTimeBar();
		//for (unsigned long i = 0; i < COUNTOF(signals); ++i) printf("%lu=%.1f ", i, signals[i]); printf("\n");

		videoOutputResize(a_app_state->width, a_app_state->height);
		videoPaint(signals, COUNTOF(signals), 1);
	}
}

static void key(ATimeUs ts, AKey key, int down) {
	(void)(ts);
	if (!down)
		return;
	switch (key) {
		case AK_Space: audioRawTogglePause(); break;
		case AK_Right: audioRawSeek(audioRawGetTimeBar() + 4.); break;
		case AK_Left: audioRawSeek(audioRawGetTimeBar() - 4.); break;
		case AK_Q: aAppTerminate(0);
		default: break;
	}
}

static void pointer(ATimeUs ts, int dx, int dy, unsigned int buttons_changed_bits) {
	(void)(ts);
	(void)(dx);
	(void)(dy);
	(void)(buttons_changed_bits);
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

	resourcesInit();

	if (!(g.project = resourceOpenFile(project))) {
		MSG("Cannot open project file %s", project);
		aAppTerminate(-1);
	}

	videoInit(width, height, "yo28.scene");

	if (1 != audioOpen(SAMPLE_RATE, 2, NULL, audioCallback, midi_device, midiCallback))
		aAppTerminate(-1);
}
