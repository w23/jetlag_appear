#include "seqgui.h"
#include "video.h"
#include "audio.h"
#include "timeline.h"
#include "fileres.h"
#include "Intro.h"

Intro::Intro(int width, int height)
	: paused_(0)
	, time_end_(1000000.f * 120.f) // * MAX_SAMPLES / (float)SAMPLE_RATE)
	, time_adjusted_(true)
	, time_(0)
	, last_frame_time_(0)
	, loop_a_(0)
	, loop_b_(time_end_)
	, mouse(aVec3ff(0))
{
	resourcesInit();
	timelineInit("timeline.seq", 44100, 120);
	audioInit("", 44100);
	videoInit(width, height);

#if 0
	float samples[440];
	audio_.synthesize(samples, 110);
	audio_.synthesize(samples + 110, 110);
	audio_.synthesize(samples + 220, 110);
	audio_.synthesize(samples + 330, 110);
	for (int i = 0; i < 440; ++i)
		printf("%f %f\n", samples[i], (i>0)?samples[i] - samples[i-1] : 0);
	aAppTerminate(0);
#endif
}

void Intro::audio(float *samples, int nsamples) {
	LFLock lock;
	const Automation *a = timelineLock(&lock);
	audioSynthesize(samples, nsamples, a);
	timelineUnlock(&lock);
}

static float fbuffer[32];

void Intro::paint(ATimeUs ts) {
	resourcesUpdate();

	if (!paused_) {
		const ATimeUs delta = ts - last_frame_time_;
		time_ += delta;
		if (time_ < loop_a_) time_ = loop_a_;
		if (time_ > loop_b_) time_ = loop_a_ + time_ % (loop_b_ - loop_a_);
	}
	last_frame_time_ = ts;

	//const float now = audio_.time();//1e-6f * time_;
	const float now = 1e-6f * time_;

	bool need_redraw = !paused_ || (midi_changed_ | time_adjusted_);
	//need_redraw |= timeline_.update();

	{
		LFLock lock;
		Frame frame;
		frame.start = 0;
		frame.end = 12;
		frame.signal = fbuffer + 3;
		fbuffer[2] = now;
		const Automation *a = timelineLock(&lock);
		automationGetSamples(a, now * 44100.f, now * 44100.f + 1, &frame);
		frame.start = 0;
		frame.end = 32;
		frame.signal = fbuffer;
		//for (int i = 0; i < frame.end; ++i) printf("%d=%f ", i, fbuffer[i]); printf("\n");
		videoPaint(&frame, need_redraw);

#if 0
		glUseProgram(0);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		const GuiRect vp = {0, 0, (int)a_app_state->width, (int)a_app_state->height};
		guiSetViewport(vp);
		GuiTransform transform = {0, 0, 1};
		{
			Timeline::ReadOnlyLock lock(timeline_);
			guiPaintAutomation(&lock.automation(), now, transform);
		}
		glDisable(GL_BLEND);
		glLoadIdentity();
#endif
	}

	midi_changed_ = false;
	time_adjusted_ = false;
}

static struct {
	int ctlid;
	int signal;
	float mul, add;
} midi_map[] = {
	-6, 13, 1.f, 0.f,
	-5, 14, 1.f, 0.f,
	2, 15, 1.f, 0.f,
	3, 16, 1.f, 0.f,
	9, 22, 1.f, 0.f,
	10, 23, 1.f, 0.f,
	11, 24, 1.f, 0.f,
	12, 25, 1.f, 0.f,
};

#define COUNTOF(c) (sizeof(c)/sizeof(*(c)))
void Intro::midiControl(int ctl, int value) {
	//if (ctl >= 0 && ctl < 4)
	//	midi_[ctl] = value / 127.f;
	for (int i = 0; i < (int)COUNTOF(midi_map); ++i) {
		if (midi_map[i].ctlid == ctl) {
			fbuffer[midi_map[i].signal+2] = value / 127.f * midi_map[i].mul + midi_map[i].add;
			break;
		}
	}

	aAppDebugPrintf("%d -> %d\n", ctl, value);
	midi_changed_ = true;
}

void Intro::adjustTime(int delta) {
	if (delta < 0 && -delta > (int)(time_ - loop_a_))
		time_ = loop_a_;
	else
		time_ += delta;
	time_adjusted_ = true;
}

void Intro::key(ATimeUs ts, AKey key) {
	(void)ts;
	switch (key) {
		case AK_Space: paused_ ^= 1; break;
		case AK_Right: adjustTime(5000000); break;
		case AK_Left: adjustTime(-5000000); break;
		case AK_Esc: aAppTerminate(0);
		default: break;
	}
}

void Intro::pointer(int dx, int dy, unsigned buttons, unsigned btn_ch) {
	(void)(dx); (void)(dy);
#if 1
	(void)buttons; (void)btn_ch;
#else
	Timeline::WriteLock lock(timeline_);
	GuiEventPointer ptr;
	ptr.x = a_app_state->pointer.x;
	ptr.y = a_app_state->pointer.y;
	ptr.button_mask = buttons;
	ptr.xor_button_mask = btn_ch;
	do {
		guiEventPointer(&lock.automation(), ptr);
	} while (!lock.unlock());
#endif
}
