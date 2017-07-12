#pragma once

#ifdef __cplusplus
extern "C" {
#endif

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

#ifdef __cplusplus
} /* extern "C" */
#endif
