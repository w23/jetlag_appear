#include "timeline.h"
#include "seqgui.h"
#define LFM_IMPLEMENT
#include "lfmodel.h"
#include "atto/app.h"
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

static struct {
	LFModel *model;
} g;

/*
static void serialize(const char *file, const Automation *a);
static void deserialize(const char *file, Automation *a);
*/

static int deserialize_old(const char *filename, Automation *a) {
	char str[1024 * 64];
	memset(str, 0, sizeof(str));
	int fd = open(filename, O_RDONLY);
	read(fd, str, sizeof(str) - 1);
	close(fd);
	const char *p = str;
	int columns, pattern_size, icol = 0, irow = 0;

#define SSCAN(expect, format, ...) { \
int n;\
if (expect != sscanf(p, format " %n", __VA_ARGS__, &n)) { \
	aAppDebugPrintf("parsing error at %d (row=%d, col=%d)", \
		p - str, irow, icol); \
	return 0; \
} \
p += n; \
}
	SSCAN(2, "%d %d", &columns, &pattern_size);
	while (*p != '\0') {
		int pat, tick, tick2;
		++irow;
		SSCAN(3, "%d:%d.%d", &pat, &tick, &tick2);
#define SAMPLES_PER_TICK 7190
#define SAMPLE_RATE 44100
		const float time = ((pat * pattern_size + tick) * 2 + tick2)
			* (float)(SAMPLES_PER_TICK) / (float)(SAMPLE_RATE) / 2.f;
		aAppDebugPrintf("row=%d time=(%dx%d:%d+%d)%f",
			irow, pat, pattern_size, tick, tick2, time);

		Point point;
		point.time = pat * 16 + tick;
		for (int i = 0; i < columns; ++i) {
			icol = i;
			SSCAN(1, "%f", point.v + i % MAX_POINT_VALUES);
			if (i % MAX_POINT_VALUES == MAX_POINT_VALUES - 1 || i == columns - 1) {
				Envelope *e = automationGetEnvelope(a, i / MAX_POINT_VALUES);
				if (e)
					envKeypointCreate(e, &point);
			}
		}
	}
#undef SSCAN

	Note *n = a->patterns[0].notes;
	int i = 0;
#define NOTE(N) \
	n[i].event = 1; n[i].num = N; i += 2; \
	n[i].event = 1; n[i].off = 1; i += 2
	NOTE(72);
	NOTE(74);
	NOTE(76);
	NOTE(77);
	NOTE(79);
	NOTE(81);
	NOTE(83);
	NOTE(84);

	return 1;
}

void timelineInit(const char *filename, int samplerate, int bpm) {
	g.model = lfmCreate(4, sizeof(Automation), NULL, malloc);

	Automation a;
	automationInit(&a, samplerate, bpm);
	deserialize_old(filename, &a);
	LFLock lock;
	lfmModifyLock(g.model, &lock);
	memcpy(lock.data_dst, &a, sizeof(a));
	ATTO_ASSERT(lfmModifyUnlock(g.model, &lock));
}

void timelineGetAutomationData(Automation *a) {
	LFLock lock;
	lfmReadLock(g.model, &lock);
	memcpy(a, lock.data_src, sizeof(*a));
	lfmReadUnlock(g.model, &lock);
}

const Automation *timelineLock(LFLock *opaque_lock) {
	lfmReadLock(g.model, opaque_lock);
	return opaque_lock->data_src;
}

void timelineUnlock(LFLock *opaque_lock) {
	lfmReadUnlock(g.model, opaque_lock);
}

void timelinePaint(const Automation *a) {
	GuiTransform transform = {0, 0, 1};
	guiPaintAutomation(a, 0 /*FIXME now*/, transform);
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
