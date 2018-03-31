#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef void(*audio_callback_f)(void *userdata, float *samples, int nsamples);
typedef void(*midi_callback_f)(void *userdata, const unsigned char *data, int bytes);
int audioOpen(int samplerate, int channels, void *userdata, audio_callback_f acb, const char *dev, midi_callback_f mcb);
void audioClose();

#if defined(AUDIO_IMPLEMENT)
#if defined(__linux__)
#include <alsa/asoundlib.h>
#include <pthread.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <stdint.h>

static struct {
	snd_pcm_t *pcm;
	snd_rawmidi_t *midi;
	audio_callback_f acb;
	midi_callback_f mcb;
	pthread_t thread;
	int should_exit;
	int channels;
} audio_;

#define AUDIO_BUFFER_SIZE 256
#define MIDI_BUFFER_SIZE 128

static void *audio_Thread(void *data) {
	// TODO priority
	while (!__atomic_load_n(&audio_.should_exit, __ATOMIC_SEQ_CST)) {
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

		audio_.acb(data, buffer, AUDIO_BUFFER_SIZE / audio_.channels);

		int err = snd_pcm_writei(audio_.pcm, buffer, AUDIO_BUFFER_SIZE / audio_.channels);
		if (err < 0) err = snd_pcm_recover(audio_.pcm, err, 0);
		if (err < 0) {
			printf("recover error: %s\n", snd_strerror(err));
			break;
		}
	}

	return NULL;
}

int audioOpen(int samplerate, int channels, void *userdata, audio_callback_f acb, const char *midi, midi_callback_f mcb) {
	int err = snd_pcm_open(&audio_.pcm, "default", SND_PCM_STREAM_PLAYBACK, 0);
	if (err < 0) {
		printf("Playback open error: %s\n", snd_strerror(err));
		return -1;
	}

	err = snd_pcm_set_params(audio_.pcm,
			SND_PCM_FORMAT_FLOAT_LE,
			SND_PCM_ACCESS_RW_INTERLEAVED,
			channels, samplerate, 1, 10000);
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

	audio_.channels = channels;
	audio_.should_exit = 0;
	audio_.acb = acb;
	audio_.mcb = mcb;
	err = pthread_create(&audio_.thread, NULL, audio_Thread, userdata);
	if (err) {
		printf("pthread_create: %d\n", err);
		return -4;
	}

	return 1;
}

void audioClose() {
	__atomic_store_n(&audio_.should_exit, 1, __ATOMIC_SEQ_CST);
	pthread_join(audio_.thread, NULL);
	snd_pcm_close(audio_.pcm);
	// TODO Close midi
}
#elif defined(_WIN32)

#include <stdlib.h>
#include <windows.h>
#include <mmsystem.h>
#include <mmreg.h>

#define NUM_BUFFERS 3
#define BUFFER_SIZE 4096

static struct {
	audio_callback_f acb;
	void *userdata;
	int channels;
	HWAVEOUT wout;
	int nhdr;
	WAVEHDR hdr[NUM_BUFFERS];
	HANDLE sem;
} audio_;

static void headerInit(WAVEHDR *hdr, int size) {
	memset(hdr, 0, sizeof(*hdr));
	hdr->lpData = malloc(size);
	hdr->dwBufferLength = size;
	waveOutPrepareHeader(audio_.wout, hdr, sizeof(*hdr));
}

static void headerDtor(WAVEHDR *hdr) {
	free(hdr->lpData);
	waveOutUnprepareHeader(audio_.wout, hdr, sizeof(*hdr));
}

void CALLBACK waveCallback(HWAVEOUT hWave, UINT uMsg, DWORD dwUser,
	DWORD dw1, DWORD dw2)
{
	if (uMsg == WOM_DONE)
	{
		ReleaseSemaphore((HANDLE)dwUser, 1, NULL);
	}
}

static DWORD WINAPI audioThread(LPVOID unused) {
	(void)unused;

	for (;;) {
		WaitForSingleObject(audio_.sem, INFINITE);

		WAVEHDR *hdr = audio_.hdr + audio_.nhdr;
		audio_.acb(audio_.userdata, (void*)hdr->lpData, hdr->dwBufferLength / audio_.channels / 4);
		waveOutWrite(audio_.wout, hdr, sizeof(*hdr));
		audio_.nhdr = (audio_.nhdr + 1) % NUM_BUFFERS;
	}
}

int audioOpen(int samplerate, int channels, void *userdata, audio_callback_f acb, const char *midi, midi_callback_f mcb) {
	audio_.userdata = userdata;
	audio_.acb = acb;
	audio_.channels = channels;

	audio_.sem = CreateSemaphore(NULL, NUM_BUFFERS, NUM_BUFFERS, NULL);

	const WAVEFORMATEX wave_fmt =
	{
		WAVE_FORMAT_IEEE_FLOAT,
		channels,
		samplerate,
		samplerate * channels * sizeof(float), // bytes per sec
		sizeof(float) * channels,             // block alignment;
		sizeof(float) * 8,             // bits per sample
		0                                    // extension not needed
	};
	waveOutOpen(&audio_.wout, WAVE_MAPPER, &wave_fmt, waveCallback, audio_.sem, CALLBACK_FUNCTION);

	for (int i = 0; i < NUM_BUFFERS; ++i)
		headerInit(audio_.hdr + i, BUFFER_SIZE);

	CreateThread(NULL, 0, audioThread, NULL, 0, NULL);

	return 1;
}

void audioClose() {
	waveOutReset(audio_.wout);
	waveOutClose(audio_.wout);
	CloseHandle(audio_.sem);
}

#else
#error NOT IMPLEMENTED
#endif

#endif /* if defined(AUDIO_IMPLEMENT) */

#ifdef __cplusplus
} /* extern "C" */
#endif
