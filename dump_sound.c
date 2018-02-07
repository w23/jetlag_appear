#include "4klang.h"
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>

static SAMPLE_TYPE sound_buffer[MAX_SAMPLES * 2];

void write_buffer(int unused) { (void)unused;
	FILE *f = fopen("sound.raw", "wb");
	fwrite(sound_buffer, 1, sizeof(sound_buffer), f);
	fclose(f);
	exit(0);
}

int main() {
	struct sigaction sa;
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = write_buffer;
	sigaction(SIGSEGV, &sa, NULL);
	__4klang_render(sound_buffer);
	write_buffer(0);
	return 0;
}
