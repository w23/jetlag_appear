#pragma once
#include "Automation.h"
#include "lfmodel.h"

#ifdef __cplusplus
extern "C" {
#endif

void timelineInit(const char *filename, int samplerate, int bpm);
const Automation *timelineLock(LFLock *opaque_lock);
void timelineUnlock(LFLock *opaque_lock);
void timelinePaintData(const Automation *a);

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
