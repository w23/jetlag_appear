#include "video.h"
#include "audio.h"
#include "timeline.h"
#include "fileres.h"
#define COUNTOF(a) (sizeof(a)/sizeof(*(a)))

#include "atto/app.h"
#define AUDIO_IMPLEMENT
#include "aud_io.h"

#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

static float fbuffer[32];

static struct {
	int ctlid;
	int signal;
	float mul, add;
} midi_map[] = {
	{-6, 12, 1.f, 0.f},
	{-5, 13, 1.f, 0.f},
	{2, 14, 1.f, 0.f},
	{3, 15, 1.f, 0.f},
	{4, 16, 1.f, 0.f},
	{5, 17, 1.f, 0.f},
	{6, 18, 1.f, 0.f},
	{7, 19, 1.f, 0.f},
	{8, 20, 1.f, 0.f},
	{9, 21, 1.f, 0.f},
	{10, 22, 1.f, 0.f},
	{11, 23, 1.f, 0.f},
	{12, 24, 1.f, 0.f},
	{13, 25, 1.f, 0.f},
	{14, 26, 1.f, 0.f},
	{15, 27, 1.f, 0.f},
	{17, 28, 1.f, 0.f},
	{18, 29, 1.f, 0.f},
};

static void midiControl(int ctl, int value) {
	//if (ctl >= 0 && ctl < 4)
	//	midi_[ctl] = value / 127.f;
	for (int i = 0; i < (int)COUNTOF(midi_map); ++i) {
		if (midi_map[i].ctlid == ctl) {
			fbuffer[midi_map[i].signal] = value / 127.f * midi_map[i].mul + midi_map[i].add;
			break;
		}
	}

	aAppDebugPrintf("%d -> %d", ctl, value);
}

static void audioCallback(void *userdata, float *samples, int nsamples) {
	(void)(userdata);
	LFLock lock;
	const Automation *a = timelineLock(&lock);
	audioSynthesize(samples, nsamples, a);
	timelineUnlock(&lock);
}

static void midiCallback(void *userdata, const unsigned char *data, int bytes) {
	(void)userdata;
	for (int i = 0; i < bytes; ++i)
		printf("%02x ", data[i]);
	printf("\n");

	for (; bytes > 2; bytes -= 3, data += 3) {
		//const int channel = data[0] & 0x0f;
		switch(data[0] & 0xf0) {
			//case 0x80:
			//case 0x90:
			//	dphase = powf(2.f, (data[1]-69) / 12.f) * 440.f / 44100.f;
			//	break;
			case 0xb0:
				midiControl(data[1] - 0x30, data[2]);
				break;
		}
	}
}

static void paint(ATimeUs ts, float dt) {
	(void)dt;
	resourcesUpdate();

	const float now = 1e-6f * ts;

	{
		LFLock lock;
		Frame frame;
		frame.start = 0;
		frame.end = 9;
		frame.signal = fbuffer + 3;
		fbuffer[2] = now;
		const Automation *a = timelineLock(&lock);
		automationGetSamples(a, now * 44100.f, now * 44100.f + 1, &frame);
		frame.start = 0;
		frame.end = 32;
		frame.signal = fbuffer;
		//for (int i = 0; i < frame.end; ++i) printf("%d=%f ", i, fbuffer[i]); printf("\n");
		videoPaint(&frame, 1);

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
	const char* midi_device = "default";
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
	timelineInit("timeline.seq", 44100, 120);
	videoInit(width, height);

	audioInit("", 44100);
	if (1 != audioOpen(NULL, audioCallback, midi_device, midiCallback))
		aAppTerminate(-1);
}
