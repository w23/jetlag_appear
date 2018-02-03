#include "common.h"

#include "atto/app.h"
#define AUDIO_IMPLEMENT
#include "aud_io.h"

#include <stdlib.h> // atoi
#include <string.h>

static struct {
	int samples, sample;
	int channels;
	int samplerate;
	float *buffer;
} audio;

static void audioCallback(void *userdata, float *samples, int nsamples) {
	(void)(userdata);
	for(int i = 0; i < nsamples; ++i) {
		samples[i] = audio.buffer[2*audio.sample];
		audio.sample += 1;
		audio.sample = audio.sample % audio.samples;
	}
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

static void paint(ATimeUs ts, float dt) {
	(void)dt;
	resourcesUpdate();

	(void)ts;
	//const float now = 1e-6f * ts;

	{
		float signals[32];
		memset(signals, 0, sizeof(signals));
		signals[0] = (float)audio.sample / audio.samplerate;
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
		/*
		case AK_Space: g.paused ^= 1; break;
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

	FILE* f = fopen("sound.raw", "rb");
	printf("%p %d\n", (void*)f, audio.samples);
	fseek(f, 0L, SEEK_END);
	printf("%p %d\n", (void*)f, audio.samples);
	audio.samples = ftell(f) / 8;
	printf("%p %d\n", (void*)f, audio.samples);
	fseek(f, 0L, SEEK_SET);
	audio.buffer = malloc(audio.samples * 8);
	fread(audio.buffer, 1, audio.samples * 8, f);
	fclose(f);
	audio.samplerate = 44100;
	audio.sample = 0;

	resourcesInit();
	videoInit(width, height);
	if (1 != audioOpen(NULL, audioCallback, midi_device, midiCallback))
		aAppTerminate(-1);
}
