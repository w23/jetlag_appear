#include "common.h"
#include "seqgui.h"
#include "Automation.h"
#define LFM_IMPLEMENT
#include "lfmodel.h"
#include "timeline.h"
#include "fileres.h"
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

static struct {
	int samplerate, bpm;
	LFModel *model;
	VolatileResource *timeline_source;
} g;

/*
static void serialize(const char *file, const Automation *a);

static void deserialize(const char *data, Automation *a) {
	automationInit(a, g.samplerate, g.bpm);
}
*/

void timelineInit(const char *filename, int samplerate, int bpm) {
	g.samplerate = samplerate;
	g.bpm = bpm;
	g.model = lfmCreate(4, sizeof(Automation), NULL, malloc);
	g.timeline_source = resourceOpenFile(filename);

	Automation a;
	automationInit(&a, samplerate, bpm);
	LFLock lock;
	lfmModifyLock(g.model, &lock);
	memcpy(lock.data_dst, &a, sizeof(a));
	ATTO_ASSERT(lfmModifyUnlock(g.model, &lock));
}

void timelineGetSignals(float *output, int signals, int count, int advance) {
	/* FIXME not implemented */
	(void)advance;
	memset(output, 0, signals * count * sizeof(float));
}

void timelinePaintUI() {
#if 0
	GuiTransform transform = {0, 0, 1};
	guiPaintAutomation(a, 0 /*FIXME now*/, transform);
#endif
}

void timelineEdit(const Event *event) {
	LFLock lock;
	lfmModifyLock(g.model, &lock);
	for (;;) {
		memcpy(lock.data_dst, lock.data_src, sizeof(Automation));

		// TODO
		(void)event;

		if (lfmModifyUnlock(g.model, &lock))
			break;
		lfmModifyRetry(g.model, &lock);
	}
}
