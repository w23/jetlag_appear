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
	, timeline_src_("timeline.seq")
	, timeline_(timeline_src_)
	, video_(width, height)
	, audio_(44100, "")
{
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
	audio_.synthesize(samples, nsamples, timeline_);
}

void Intro::paint(ATimeUs ts) {
	if (!paused_) {
		const ATimeUs delta = ts - last_frame_time_;
		time_ += delta;
		if (time_ < loop_a_) time_ = loop_a_;
		if (time_ > loop_b_) time_ = loop_a_ + time_ % (loop_b_ - loop_a_);
	}
	last_frame_time_ = ts;

	const float now = 1e-6f * time_;

	bool need_redraw = !paused_ || (midi_changed_ | time_adjusted_);
	need_redraw |= timeline_.update();

	video_.paint(timeline_, now, need_redraw);

	midi_changed_ = false;
	time_adjusted_ = false;
}

void Intro::midiControl(int ctl, int value) {
	if (ctl >= 0 && ctl < 4)
		midi_[ctl] = value / 127.f;
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
	if (buttons) {
		mouse.x += dx;
		mouse.y += dy;
	}

	if (btn_ch & AB_WheelUp) mouse.z += 1.f;
	if (btn_ch & AB_WheelDown) mouse.z -= 1.f;
}
