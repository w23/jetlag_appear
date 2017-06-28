#pragma once

#include "FileString.h"
#include "Automation.h"
#include "lfmodel.h"

class Timeline {
public:
	Timeline(String &source, int samplerate, int bpm);
	~Timeline();
	
	bool update();

	struct ReadOnlyLock {
		ReadOnlyLock(const Timeline &timeline);
		~ReadOnlyLock();
		const Automation &automation() const;
	
	private:
		LFLock lock_;
		LFModel &model_;
	};

	struct WriteLock {
		WriteLock(Timeline &timeline);
		bool unlock();
		~WriteLock();
		Automation &automation();
	
	private:
		LFLock lock_;
		LFModel &model_;
	};

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
	friend struct ReadOnlyLock;
	friend struct WriteLock;

	String &source_;
	const int samplerate_, bpm_;

	LFModel *model_;

	bool parse(const char *str, Automation *a);
};
