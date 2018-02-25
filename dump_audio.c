#include "4klang.h"
#include <string.h>
#include <stdio.h>
#ifdef __linux__
#include <signal.h>
#endif
#include <stdlib.h>

static SAMPLE_TYPE sound_buffer[MAX_SAMPLES * 2];

const char *filename = "audio.raw";

void write_buffer() {
	printf("Writing\n");
	FILE *f = fopen(filename, "wb");
	fwrite(sound_buffer, 1, sizeof(sound_buffer), f);
	fclose(f);
	printf("Done\n");
	exit(0);
}

void handleCrash(int unused) { (void)unused;
	printf("Crash detected :(\n");
	write_buffer();
}

int main(int argc, char *argv[]) {
	if (argc > 1)
		filename = argv[1];

	printf("Generating %d samples (%d seconds)... (will take a minute)\n",
		MAX_SAMPLES, MAX_SAMPLES * 2 / SAMPLE_RATE);

#ifdef __linux__
	struct sigaction sa;
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = handleCrash;
	sigaction(SIGSEGV, &sa, NULL);
#endif
	__4klang_render(sound_buffer);
	write_buffer();
	return 0;
}
