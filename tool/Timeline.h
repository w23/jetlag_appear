#pragma once

#include "FileString.h"
#include <vector>
#include <cstdio>

class Timeline {
public:
	Timeline(String &source);
	
	bool update();

	struct Sample {
		float operator[](int index) const {
			if (index < 0 || (size_t)index >= data_.size()) {
				//aAppDebugPrintf("index %d is out of bounds [%d, %d]",
				//	index, 0, static_cast<int>(data_.size()) - 1);
				return 0;
			}

			return data_[index];
		}

		std::vector<float> data_;
	};

	Sample sample(float time) const;

private:
	String &source_;

	int columns_, rows_;
	std::vector<float> data_;

	bool parse(const char *str);
};
