#include "common.h"

#include "atto/app.h"
#define AUDIO_IMPLEMENT
#include "aud_io.h"

#include <stdlib.h> // atoi
#include <string.h>

static void audioCallback(void *userdata, float *samples, int nsamples) {
	(void)(userdata);
	audioSynthesize(samples, nsamples);
}

static void midiCallback(void *userdata, const unsigned char *data, int bytes) {
	(void)userdata;

	for (; bytes > 2; bytes -= 3, data += 3) {
		//const int channel = data[0] & 0x0f;
		switch(data[0] & 0xf0) {
			case 0x80:
			case 0x90:
				timelineMidiNote(data[1], data[2], !!(data[0] & 0x10));
				break;
			case 0xb0:
				timelineMidiCtl(data[1], data[2]);
				break;
			default:
				MSG("%02x %02x %02x", data[0], data[1], data[2]);
		}
	}
}

static void paint(ATimeUs ts, float dt) {
	(void)dt;
	resourcesUpdate();
	timelineCheckUpdate();
	audioCheckUpdate();

	(void)ts;
	//const float now = 1e-6f * ts;

	{
		float signals[32];
		memset(signals, 0, sizeof(signals));
		timelineGetLatestSignals(signals, COUNTOF(signals));
		//for (unsigned long i = 0; i < COUNTOF(signals); ++i) printf("%lu=%.1f ", i, signals[i]); printf("\n");

		videoOutputResize(a_app_state->width, a_app_state->height);
		videoPaint(signals, COUNTOF(signals), 1);

#if 0
		glUseProgram(0);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		const GuiRect vp = {0, 0, (int)a_app_state->width, (int)a_app_state->height};
		guiSetViewport(vp);
		GuiTransform transform = {0, 0, 1};
		{
			Timeline::ReadOnlyLock lock(timeline_);
			guiPaintAutomation(&lock.automation(), now, transform);
		}
		glDisable(GL_BLEND);
		glLoadIdentity();
#endif
	}
}

static void key(ATimeUs ts, AKey key, int down) {
	(void)(ts);
	if (!down)
		return;
	switch (key) {
	//	case AK_Space: g.paused ^= 1; break;
/*
		case AK_Right: adjustTime(5000000); break;
		case AK_Left: adjustTime(-5000000); break;
*/
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
	timelineInit("timeline.txt", 44100);
	videoInit(width, height);

	audioInit("synth.src", 44100);
	if (1 != audioOpen(NULL, audioCallback, midi_device, midiCallback))
		aAppTerminate(-1);
}
