//#include "music/4klang.h"

#include "atto/app.h"
#define AUDIO_IMPLEMENT
#include "audio.h"

#include <utility>
#include <memory>
#include <vector>

#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>

#include "Intro.h"

static std::unique_ptr<Intro> intro;

static void audioCallback(void *userdata, float *samples, int nsamples) {
	Intro *intro = static_cast<Intro*>(userdata);
	intro->audio(samples, nsamples);
}

static void midiCallback(void *userdata, const unsigned char *data, int bytes) {
	for (int i = 0; i < bytes; ++i)
		printf("%02x ", data[i]);
	printf("\n");

	Intro *intro = static_cast<Intro*>(userdata);

	for (; bytes > 2; bytes -= 3, data += 3) {
		//const int channel = data[0] & 0x0f;
		switch(data[0] & 0xf0) {
			//case 0x80:
			//case 0x90:
			//	dphase = powf(2.f, (data[1]-69) / 12.f) * 440.f / 44100.f;
			//	break;
			case 0xb0:
				intro->midiControl(data[1] - 0x30, data[2]);
				break;
		}
	}
}

void paint(ATimeUs ts, float dt) {
	(void)(dt);
	intro->paint(ts);
}

void key(ATimeUs ts, AKey key, int down) {
	(void)(ts);
	if (down) intro->key(ts, key);
}

void pointer(ATimeUs ts, int dx, int dy, unsigned int buttons_changed_bits) {
	(void)(ts);
	(void)(dx);
	(void)(dy);
	(void)(buttons_changed_bits);
	intro->pointer(dx, dy, a_app_state->pointer.buttons, buttons_changed_bits);
}

void attoAppInit(struct AAppProctable *ptbl) {
	ptbl->key = key;
	ptbl->pointer = pointer;
	ptbl->paint = paint;

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

	intro.reset(new Intro(width, height));
	audioOpen(intro.get(), audioCallback, midi_device, midiCallback);
}
