#pragma once 

typedef void(*audio_callback_f)(void *userdata, float *samples, int nsamples);
typedef void(*midi_callback_f)(void *userdata, const unsigned char *data, int bytes);
int audioOpen(void *userdata, audio_callback_f acb, const char *dev, midi_callback_f mcb);
void audioClose();

#if defined(AUDIO_IMPLEMENT)
#if defined(__linux__)
#include <alsa/asoundlib.h>
#include <pthread.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <stdint.h>

uint64_t timeNow() {
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
	return ts.tv_sec * 1000000ull + ts.tv_nsec / 1000ull;
}

static struct {
	snd_pcm_t *pcm;
	snd_rawmidi_t *midi;
	audio_callback_f acb;
	midi_callback_f mcb;
	pthread_t thread;
} audio_;

#define AUDIO_BUFFER_SIZE 256
#define MIDI_BUFFER_SIZE 128

static void *audio_Thread(void *data) {
	// TODO priority
	for(;;) {
		float buffer[AUDIO_BUFFER_SIZE];
		unsigned char midibuf[MIDI_BUFFER_SIZE];

		if (audio_.midi && audio_.mcb) {
			int err = snd_rawmidi_read(audio_.midi, midibuf, sizeof(midibuf));
			if (err < 0 && err != -EAGAIN) {
				printf("midi_read: %s\n", snd_strerror(err));
				return NULL;
			}

			if (err > 0)
				audio_.mcb(data, midibuf, err);
		}

		audio_.acb(data, buffer, AUDIO_BUFFER_SIZE);

		int err = snd_pcm_writei(audio_.pcm, buffer, AUDIO_BUFFER_SIZE);
		if (err < 0) err = snd_pcm_recover(audio_.pcm, err, 0);
		if (err < 0) {
			printf("recover error: %s\n", snd_strerror(err));
			return NULL;
		}
	}
}

int audioOpen(void *userdata, audio_callback_f acb, const char *midi, midi_callback_f mcb) {
	int err = snd_pcm_open(&audio_.pcm, "default", SND_PCM_STREAM_PLAYBACK, 0);
	if (err < 0) {
		printf("Playback open error: %s\n", snd_strerror(err));
		return -1;
	}

	err = snd_pcm_set_params(audio_.pcm,
			SND_PCM_FORMAT_FLOAT_LE,
			SND_PCM_ACCESS_RW_INTERLEAVED,
			1, 44100, 1, 10000);
	if (err < 0) {
		printf("set_params error: %s\n", snd_strerror(err));
		return -2;
	}

	if (mcb && midi && midi[0] != '\0') {
		err = snd_rawmidi_open(&audio_.midi, NULL, midi, SND_RAWMIDI_NONBLOCK);
		if (err < 0) {
			printf("rawmidi_open: %s\n", snd_strerror(err));
			return -3;
		}
	} else {
		audio_.midi = NULL;
	}

	audio_.acb = acb;
	audio_.mcb = mcb;
	pthread_create(&audio_.thread, NULL, audio_Thread, userdata);

	return 1;
}

void audioClose() {
	snd_pcm_close(audio_.pcm);
}
#else
#error NOT IMPLEMENTED
#endif
#endif
