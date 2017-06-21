#pragma once

#include "FileString.h"
#include "Automation.h"

typedef struct LFModel LFModel;

class Timeline {
public:
	Timeline(String &source, int samplerate, int bpm);
	~Timeline();
	
	bool update();

	struct Sample {
		float operator[](int index) const {
			if (index < 0 || index >= SAMPLE_SIGNALS) {
				//aAppDebugPrintf("index %d is out of bounds [%d, %d]",
				//	index, 0, static_cast<int>(data_.size()) - 1);
				return 0;
			}

			return frame.signal[index];
		}

		Frame frame;
	};

	Sample sample(float time) const;

private:
	String &source_;
	const int samplerate_, bpm_;

	LFModel *model_;

	bool parse(const char *str, Automation *a);
};
