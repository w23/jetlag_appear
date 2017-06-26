#pragma once

class Timeline;

class Audio {
public:
	Audio(int samplerate, const char *audio_file);
	void synthesize(float *samples, int num_samples, const Timeline &);

	float time() const { return time_; }

private:
	const int samplerate_;
	float state_[2];
	unsigned long samples_;
	float time_;
}; // class Audio
