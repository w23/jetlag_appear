#pragma once

#include <stdint.h>

#define COUNTOF(a) (sizeof(a) / sizeof(*(a)))

#define ASSERT(c) (void)(c)

#define MSG(...) \
	aAppDebugPrintf( __VA_ARGS__)

#ifdef __cplusplus
extern "C" {
#endif

void aAppDebugPrintf(const char *fmt, ...);

typedef struct VolatileResource_t {
	int updated;
	const void *bytes;
	int size;
} VolatileResource;

VolatileResource *resourceOpenFile(const char *filename);
void resourceClose(VolatileResource *handle);
void resourcesInit();
void resourcesUpdate();

void videoInit(int width, int height);
void videoOutputResize(int width, int height);
void videoPaint(float *signals, int num_signals, int force_redraw);

void audioInit(const char *synth_src, int samplerate);
void audioSynthesize(float *samples, int num_samples);
void audioCheckUpdate();

#define MAX_DIAG_SIGNALS 16
#define MAX_DIAG_SAMPLES 65536
typedef struct {
	unsigned writepos;
	struct {
		float f[MAX_DIAG_SAMPLES];
	} signal[MAX_DIAG_SIGNALS];
} DiagSignals;
extern DiagSignals diag_signals;

void timelineInit(const char *filename, int samplerate);
void timelineCheckUpdate();
void timelinePaintUI();
void timelineGetLatestSignals(float *output, int signals);
void timelineComputeSignalsAndAdvance(float *output, int signals, int count);

typedef struct {
	enum {
		Event_MouseDownLeft,
		Event_MouseDownMiddle,
		Event_MouseDownRight,
		Event_MouseUpLeft,
		Event_MouseUpMiddle,
		Event_MouseUpRight,
		Event_MouseMove,
		Event_KeyDown,
		Event_KeyUp
	} type;
	int x, y, dx, dy;
} Event;
void timelineEdit(const Event *event);

#define PARSER_MAX_TOKEN_LENGTH 16
#define PARSER_MAX_LINE_TOKENS 8

typedef struct {
	const char *line, *prev_line;
	int line_number;
	struct {
		char str[PARSER_MAX_TOKEN_LENGTH];
	} token[PARSER_MAX_LINE_TOKENS];
	int tokens;
	enum {
		Status_Parsed,
		Status_End,
		Status_TokenTooLong,
		Status_TooManyTokens,
	} status;
} ParserContext;

void parseLine(ParserContext *context);

#define PARSER_MAX_ARGS 16
typedef enum {
	PAF_None = 0,
	PAF_Int = 1<<0,
	PAF_Float = 1<<1,
	PAF_String = 1<<2,
	PAF_Var = 1<<3,
} ParserArgTypeFlag;
typedef struct {
	ParserArgTypeFlag type;
	const char *s;
	int slen;
	union {
		int i;
		float f;
	} value;
} ParserArg;

typedef struct ParserLine {
	const char *name;
	int min_args, max_args;
	struct {
		unsigned type_flags;
	} args[PARSER_MAX_ARGS];
	int (*callback)(const struct ParserLine *, const ParserArg *args, int nargs, void *userdata);
} ParserLine;

typedef struct {
	const char *next_line;
	ParserLine *line_table;
	int line_table_length;
	void *userdata;
} ParserTokenizer;

int parserTokenizeLine(ParserTokenizer *tokenizer);

static inline uint32_t rng(uint64_t *state) {
	*state = 1442695040888963407ull + (*state) * 6364136223846793005ull;
	return (*state) >> 32;
}

#if defined(__GNUC__)
#define MEMORY_BARRIER() __atomic_thread_fence(__ATOMIC_SEQ_CST)
#define ATOMIC_FETCH(value) __atomic_load_n(&(value), __ATOMIC_SEQ_CST)
#define ATOMIC_ADD_AND_FETCH(value, add) __atomic_add_fetch(&(value), add, __ATOMIC_SEQ_CST)
#define ATOMIC_CAS(value, expect, replace) \
	__atomic_compare_exchange_n(&(value), &(expect), replace, 0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST)
#elif defined(_MSC_VER)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define NOMSG
#include <windows.h>
#define MEMORY_BARRIER() MemoryBarrier()
#define ATOMIC_FETCH(value) InterlockedOr(&(value), 0)
static inline long __atomic_add_fetch(long volatile *value, long add) {
	for (;;) {
		const long expect = ATOMIC_FETCH(*value);
		if (expect == InterlockedCompareExchange(value, expect + add, expect))
			return expect + add;
	}
}
#define ATOMIC_ADD_AND_FETCH(value, add) __atomic_add_fetch(&(value), add)
#define ATOMIC_CAS(value, expect, replace) (expect == InterlockedCompareExchange(&(value), replace, expect))
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif
