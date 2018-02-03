#include "4klang.h"
#include <stdio.h>

static SAMPLE_TYPE sound_buffer[MAX_SAMPLES * 2];
int main() {
	__4klang_render(sound_buffer);
	FILE *f = fopen("sound.raw", "wb");
	fwrite(sound_buffer, 1, sizeof(sound_buffer), f);
	fclose(f);
	return 0;
}
