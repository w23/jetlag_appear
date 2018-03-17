#pragma once

#include <stdint.h>
#include <string.h>

#include "atto/app.h"
#include "atto/math.h"

#define COUNTOF(a) (sizeof(a) / sizeof(*(a)))

#define ASSERT(c) (void)(c)

#define MSG(...) \
	aAppDebugPrintf( __VA_ARGS__)

#ifdef __cplusplus
extern "C" {
#endif

void aAppDebugPrintf(const char *fmt, ...);

char *strMakeCopy(const char *str);

typedef struct {
	const char *str;
	int length;
} StringView;

static inline StringView stringView(const char *str, int length) {
	StringView sv = { str, length }; return sv;
}

static inline StringView stringViewZ(const char *str) {
	StringView sv = { str, strlen(str) }; return sv;
}

typedef struct {
	char *str;
	int length;
	int capacity;
} MutableString;

void mutableStringInit(MutableString *ms);
void mutableStringAppendSV(MutableString *ms, StringView sv);
void mutableStringAppend(MutableString *ms, const char *, int length);
void mutableStringAppendZ(MutableString *ms, const char *);
void mutableStringDestroy(MutableString *ms);
char *mutableStringRelease(MutableString *ms);

typedef struct VolatileResource_t {
	int updated;
	int sequence;
	const void *bytes;
	int size;
} VolatileResource;

VolatileResource *resourceOpenFile(const char *filename);
void resourceClose(VolatileResource *handle);
void resourcesInit();
void resourcesUpdate();

int videoInit(int width, int height, const char *filename);
void videoOutputResize(int width, int height);
void videoPaint();

typedef enum {
	VarType_None,
	VarType_Float,
	VarType_Vec2,
	VarType_Vec3,
	VarType_Vec4
} VarType;
typedef struct {
#define MAX_VARIABLE_NAME_LENGTH 31
	char name[MAX_VARIABLE_NAME_LENGTH + 1];
	VarType type;
	// TODO:
	// - semantic
	// - range
	// - interpolation
} VarDesc;

typedef struct {
	int bar, tick;
} Timecode;

VarType varGetType(StringView sv);
const char *varGetTypeName(VarType type);

void varInit(const char *filename);
void varFrame(float bar);
int varGet(const VarDesc *desc, AVec4f *value);

typedef struct {
	enum {
		Input_Key,
		Input_Pointer,
		Input_MidiCtl,
	} type;
	union {
		struct {
			AKey code;
			int down;
		} key;
		struct {
			int x, y, dx, dy;
			int buttons;
			int buttons_xor;
		} pointer;
	} e;
} ToolInputEvent;

typedef enum {
	ToolResult_Ignored,
	ToolResult_Consumed,
	ToolResult_Released
} ToolResult;

typedef struct Tool {
	void (*activate)(struct Tool *tool);
	void (*deactivate)(struct Tool *tool);
	ToolResult (*processEvent)(struct Tool *tool, const ToolInputEvent *event);
	void (*update)(struct Tool *t, float dt);
} Tool;

typedef struct {
	Tool *camera;
} VarTools;

extern VarTools var_tools;

/*
void audioInit(const char *synth_src, int samplerate);
void audioSynthesize(float *samples, int num_samples);
void audioCheckUpdate();
*/

int audioRawInit(const char *filename, int samplerate, int channels, int bpm);
void audioRawWrite(float *samples, int num_samples);
int audioRawTogglePause();
void audioRawSeek(float bar);
float audioRawGetTimeBar();

/*
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
void timelineGetLatestSignals(float *output, int signals);
void timelineComputeSignalsAndAdvance(float *output, int signals, int count);
void timelineMidiCtl(int ctl, int value);
void timelineMidiNote(int note, int vel, int on);
void timelinePaintUI();

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
*/

#define PARSER_MAX_TOKEN_LENGTH 32
#define PARSER_MAX_LINE_TOKENS 32

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
	PAF_Var = 1<<2,
	PAF_Time = 1<<3,
	PAF_String = 1<<4,
	PAF__MAX
} ParserArgTypeFlag;

typedef struct {
	ParserArgTypeFlag type;
	const char *s;
	int slen;
	union {
		int i;
		float f;
		Timecode time;
	} value;
} ParserArg;

typedef struct {
	const struct ParserLine *line;
	const ParserArg *args;
	int num_args;
	void *userdata;
	int line_param0;
} ParserCallbackParams;

typedef struct ParserLine {
	const char *name;
	int param0;
	int min_args, max_args;
	unsigned args_flags[PARSER_MAX_ARGS];
	int (*callback)(const ParserCallbackParams *params);
} ParserLine;

typedef struct {
	ParserContext ctx;
	const ParserLine *line_table;
	int line_table_length;
	void *userdata;
} ParserTokenizer;

enum {
	Tokenize_Parsed,
	Tokenize_End,
	Tokenize_TokenTooLong,
	Tokenize_TooManyTokens,
	Tokenize_TokenWrongFormat,
};

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
