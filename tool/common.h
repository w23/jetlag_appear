#pragma once

#define COUNTOF(a) (sizeof(a) / sizeof(*(a)))

#define ASSERT(c)

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

void timelineInit(const char *filename, int samplerate, int bpm);
void timelinePaintUI();
void timelineGetSignals(float *output, int signals, int count, int advance);

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

#ifdef __cplusplus
} /* extern "C" */
#endif
