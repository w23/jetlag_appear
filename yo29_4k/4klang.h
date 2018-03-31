// some useful song defines for 4klang
#define SAMPLE_RATE 44100
#define BPM 60.000000
#define MAX_INSTRUMENTS 11
#define MAX_PATTERNS 57
#define PATTERN_SIZE_SHIFT 4
#define PATTERN_SIZE (1 << PATTERN_SIZE_SHIFT)
#define MAX_TICKS (MAX_PATTERNS*PATTERN_SIZE)
#define SAMPLES_PER_TICK 11025
#define MAX_SAMPLES (SAMPLES_PER_TICK*MAX_TICKS)
#define POLYPHONY 2
#define FLOAT_32BIT
#define SAMPLE_TYPE float

#define WINDOWS_OBJECT

void  __stdcall	_4klang_render(void*);
