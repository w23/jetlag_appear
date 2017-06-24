#include "Timeline.h"
#define LFM_IMPLEMENT
#include "lfmodel.h"
#include "atto/app.h"
#include <cstdio>

Timeline::Timeline(String &source, int samplerate, int bpm)
	: source_(source)
	, samplerate_(samplerate)
	, bpm_(bpm)
	, model_(lfmCreate(4, sizeof(Automation), nullptr, malloc))
{
	Automation a;
	automationInit(&a, samplerate, bpm);
	LFLock lock;
	lfmModifyLock(model_, &lock);
	for(;;) {
		memcpy(lock.data_dst, &a, sizeof(a));
		if (lfmModifyUnlock(model_, &lock))
			break;
		lfmModifyRetry(model_, &lock);
	}
}

Timeline::~Timeline()
{
	free(model_);
}

bool Timeline::parse(const char *str, Automation *a) {
	const char *p = str;
	int columns, pattern_size, icol = 0, irow = 0;

#define SSCAN(expect, format, ...) { \
int n;\
if (expect != sscanf(p, format " %n", __VA_ARGS__, &n)) { \
	aAppDebugPrintf("parsing error at %d (row=%d, col=%d)", \
		p - str, irow, icol); \
	return false; \
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

	return true;
}

bool Timeline::update() {
	if (source_.update()) {
		Automation a;
		automationInit(&a, samplerate_, bpm_);
		if (!parse(source_.string().c_str(), &a))
			return false;

		Note *n = a.patterns[0].notes;
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

		LFLock lock;
		lfmModifyLock(model_, &lock);
		for(;;) {
			memcpy(lock.data_dst, &a, sizeof(a));
			if (lfmModifyUnlock(model_, &lock))
				break;
			lfmModifyRetry(model_, &lock);
		}

		return true;
	}
	return false;
}

Timeline::Sample Timeline::sample(float time) const {
	LFLock lock;
	lfmReadLock(model_, &lock);
	const Automation *a = static_cast<const Automation*>(lock.data_src);

	Timeline::Sample ret;
	automationGetSamples(a, /*FIXME*/time * a->samplerate, time * a->samplerate + 1, &ret.frame);

	lfmReadUnlock(model_, &lock);

	return ret;
}
