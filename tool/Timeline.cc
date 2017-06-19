#include "Timeline.h"

Timeline::Timeline(String &source) : source_(source), columns_(0), rows_(0) {}

bool Timeline::parse(const char *str) {
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
	std::vector<float> data;
	float lasttime = 0;
	while (*p != '\0') {
		int pat, tick, tick2;
		++irow;
		SSCAN(3, "%d:%d.%d", &pat, &tick, &tick2);
#define SAMPLES_PER_TICK 7190
#define SAMPLE_RATE 44100
		const float time = ((pat * pattern_size + tick) * 2 + tick2)
			* (float)(SAMPLES_PER_TICK) / (float)(SAMPLE_RATE) / 2.f;
		data.push_back(time - lasttime);
		aAppDebugPrintf("row=%d time=(%dx%d:%d+%d)%f, dt=%f",
			irow, pat, pattern_size, tick, tick2, time, data.back());
		lasttime = time;

		for (int i = 0; i < columns; ++i) {
			float value;
			icol = i;
			SSCAN(1, "%f", &value);
			data.push_back(value);
		}
	}
#undef SSCAN

	rows_ = irow - 1;
	columns_ = columns;
	data_ = std::move(data);
	return true;
}

bool Timeline::update() {
	return source_.update() && parse(source_.string().c_str());
}

Timeline::Sample Timeline::sample(float time) const {
	int i, j;
	for (i = 1; i < rows_ - 1; ++i) {
		const float dt = data_.at(i * (columns_ + 1));
		//printf("%.2f: [i=%d; dt=%.2f, t=%.2f]\n", time, i, dt, time/dt);
		if (dt >= time) {
			time /= dt;
			break;
		}
		time -= dt;
	}

	Sample ret;
	ret.data_.resize(columns_);

	for (j = 0; j < columns_; ++j) {
		const float a = data_.at((i - 1) * (columns_ + 1) + j + 1);
		const float b = data_.at(i * (columns_ + 1) + j + 1);
		ret.data_[j] = a + (b - a) * time;
		//printf("%.2f ", ret.data_[j]);
	}
	//printf("\n");

	return ret;
}
