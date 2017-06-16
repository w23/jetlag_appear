#pragma once

struct op_t {
	enum {
		OP_PUSH,
		//OP_POP,
		OP_PUSH_STATE,
		OP_POP_STATE,
		OP_PUSH_IN,
		OP_DUP,
		OP_ADD,
		OP_MUL,
		OP_PSINE,
		OP_PTRI,
		OP_FRACT
	} op;
	struct /* FIXME union */ {
		int i;
		float f;
	} arg;
};

struct machine_t {
	int n_inputs, n_state;
	float *state;
	const op_t *program;
	int program_size;
};

class Audio {
public:
	Audio(int samplerate, const char *audio_file);
	void synthesize(float *samples, int num_samples /*Timeline*/);

private:
	const int samplerate_;
	float state_[2];
	machine_t machine_;
}; // class Audio
