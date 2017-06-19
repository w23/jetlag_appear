#pragma once

class Audio {
public:
	Audio(int samplerate, const char *audio_file);
	void synthesize(float *samples, int num_samples /*Timeline*/);

private:
	const int samplerate_;
	float state_[2];
}; // class Audio
