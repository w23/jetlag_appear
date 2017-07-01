#pragma once
#include "FileString.h"

#include "atto/math.h"
#include "atto/app.h"

#include "Timeline.h"
#include "Audio.h"

class Intro {
	int paused_;
	const ATimeUs time_end_;
	bool time_adjusted_;
	ATimeUs time_, last_frame_time_;
	ATimeUs loop_a_, loop_b_;

	AVec3f mouse;

	FileString timeline_src_;
	Timeline timeline_;

	Audio audio_;

	float midi_[4];
	bool midi_changed_;

public:
	Intro(int width, int height);
	void audio(float *samples, int nsamples);
	void paint(ATimeUs ts);
	void midiControl(int ctl, int value);
	void adjustTime(int delta);
	void key(ATimeUs ts, AKey key);
	void pointer(int dx, int dy, unsigned buttons, unsigned btn_ch);
};
