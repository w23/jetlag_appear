#pragma once
#include "FileString.h"

#include "atto/math.h"
#include "atto/app.h"
#define ATTO_GL_H_IMPLEMENT
#include "atto/gl.h"

#include "Timeline.h"

const char fs_vtx_source[] =
	"void main() {\n"
		"gl_Position = gl_Vertex;\n"
	"}"
;

class Program {
	String &vertex_src_;
	String &fragment_src_;
	AGLProgram program_;

public:
	const AGLProgram& program() const { return program_; }
	Program(String &vtx, String &frg) : vertex_src_(vtx), fragment_src_(frg), program_(0) {}

	bool update() {
		if (vertex_src_.update() || fragment_src_.update()) {
			AGLProgram new_program = aGLProgramCreateSimple(
					vertex_src_.string().c_str(), fragment_src_.string().c_str());
			if (new_program < 0) {
				aAppDebugPrintf("shader error: %s\n", a_gl_error);
				return false;
			}

			if (program_ > 0)
				aGLProgramDestroy(program_);

			program_ = new_program;
			return true;
		}
		return false;
	}
};

class Intro {
	int paused_;
	const ATimeUs time_end_;
	bool time_adjusted_;
	ATimeUs time_, last_frame_time_;
	ATimeUs loop_a_, loop_b_;

	AVec3f cam_, at_;
	AVec3f mouse;

	int frame_width, frame_height;

	String fs_vtx;
	FileString raymarch_src;
	Program raymarch_prg;
	FileString post_src;
	Program post_prg;
	FileString out_src;
	Program out_prg;

	FileString timeline_src_;
	Timeline timeline_;

	float midi_[4];
	bool midi_changed_;

	enum {
		FbTex_None,
		FbTex_Random,
		FbTex_Ray,
		FbTex_Frame,
		FbTex_COUNT
	};
	GLuint tex_[FbTex_COUNT];
	GLuint fb_[FbTex_COUNT];

	static void createTexture(GLint t, int w, int h)
	{
		AGL__CALL(glBindTexture(GL_TEXTURE_2D, t));
		AGL__CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, w, h, 0, GL_RGBA, GL_FLOAT, 0));
		AGL__CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
		AGL__CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
		AGL__CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER));
		AGL__CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER));
	}

public:
	Intro(int width, int height)
		: paused_(0)
		, time_end_(1000000.f * 120.f) // * MAX_SAMPLES / (float)SAMPLE_RATE)
		, time_(0)
		, time_adjusted_(true)
		, last_frame_time_(0)
		, loop_a_(0)
		, loop_b_(time_end_)
		, cam_(aVec3f(40.f, 1.5f, 0.f))
		, at_(aVec3ff(0.f))
		, mouse(aVec3ff(0))
		, frame_width(width)
		, frame_height(height)
		, fs_vtx(fs_vtx_source)
		, raymarch_src("raymarch.glsl")
		, raymarch_prg(fs_vtx, raymarch_src)
		, post_src("post.glsl")
		, post_prg(fs_vtx, post_src)
		, out_src("out.glsl")
		, out_prg(fs_vtx, out_src)
		, timeline_src_("timeline.seq")
		, timeline_(timeline_src_)
	{
		tex_[0] = fb_[0] = 0;
		AGL__CALL(glGenTextures(FbTex_COUNT-1, tex_ + 1));
		AGL__CALL(glGenFramebuffers(FbTex_COUNT-1, fb_ + 1));

		for (int i = 0; i < FbTex_COUNT; ++i)
			aAppDebugPrintf("tex_[%d] = %u; fb_[%d] = %u;", i, tex_[i], i, fb_[i]);

		createTexture(tex_[FbTex_Ray], frame_width, frame_height);
		AGL__CALL(glBindFramebuffer(GL_FRAMEBUFFER, fb_[FbTex_Ray]));
		AGL__CALL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex_[FbTex_Ray], 0));
		int status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		ATTO_ASSERT(status == GL_FRAMEBUFFER_COMPLETE);

		createTexture(tex_[FbTex_Frame], frame_width, frame_height);
		AGL__CALL(glBindFramebuffer(GL_FRAMEBUFFER, fb_[FbTex_Frame]));
		AGL__CALL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex_[FbTex_Frame], 0));
		status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		ATTO_ASSERT(status == GL_FRAMEBUFFER_COMPLETE);
	}

	void drawPass(float now, const Timeline::Sample &TV, int tex, int prog, int fb) {
		AGL__CALL(glBindTexture(GL_TEXTURE_2D, tex));
		AGL__CALL(glUseProgram(prog));
		AGL__CALL(glBindFramebuffer(GL_FRAMEBUFFER, fb));
		if (fb > 0) {
			AGL__CALL(glViewport(0, 0, frame_width, frame_height));
			AGL__CALL(glUniform3f(glGetUniformLocation(prog, "V"), frame_width, frame_height, now));
		} else {
			AGL__CALL(glViewport(0, 0, a_app_state->width, a_app_state->height));
			AGL__CALL(glUniform3f(glGetUniformLocation(prog, "V"), a_app_state->width, a_app_state->height, now));
		}
		AGL__CALL(glUniform1i(glGetUniformLocation(prog, "B"), 0));
		AGL__CALL(glUniform3f(glGetUniformLocation(prog, "C"), TV[0], TV[1], TV[2]));
		AGL__CALL(glUniform3f(glGetUniformLocation(prog, "A"), TV[3], TV[4], TV[5]));
		AGL__CALL(glUniform3f(glGetUniformLocation(prog, "D"), TV[6], TV[7], TV[8]));
		AGL__CALL(glUniform4f(glGetUniformLocation(prog, "M"), midi_[0], midi_[1], midi_[2], midi_[3]));

		AGL__CALL(glUniform1f(glGetUniformLocation(prog, "TPCT"), (float)time_ / (float)time_end_));
		AGL__CALL(glRects(-1,-1,1,1));
	}

	void paint(ATimeUs ts) {
		if (!paused_) {
			const ATimeUs delta = ts - last_frame_time_;
			time_ += delta;
			if (time_ < loop_a_) time_ = loop_a_;
			if (time_ > loop_b_) time_ = loop_a_ + time_ % (loop_b_ - loop_a_);
		}
		last_frame_time_ = ts;

		const float now = 1e-6f * time_;

		cam_.x = 20.f + sinf(now*.1f) * 18.f;

		bool need_redraw = !paused_ || (midi_changed_ | time_adjusted_);
		need_redraw |= raymarch_prg.update();
		need_redraw |= post_prg.update();
		need_redraw |= out_prg.update();
		need_redraw |= timeline_.update();

		midi_changed_ = false;

		const Timeline::Sample TV = timeline_.sample(now);

		if (need_redraw) {
			drawPass(now, TV, 0, raymarch_prg.program(), FbTex_Ray);
			drawPass(now, TV, FbTex_Ray, post_prg.program(), FbTex_Frame);
		}
		drawPass(now, TV, FbTex_Frame, out_prg.program(), 0);

		time_adjusted_ = false;
	}

	void midiControl(int ctl, int value) {
		if (ctl >= 0 && ctl < 4)
			midi_[ctl] = value / 127.f;
		printf("%d -> %d\n", ctl, value);
		midi_changed_ = true;
	}

	void adjustTime(int delta) {
		if (delta < 0 && -delta > (int)(time_ - loop_a_))
			time_ = loop_a_;
		else
			time_ += delta;
		time_adjusted_ = true;
	}

	void key(ATimeUs ts, AKey key) {
		(void)ts;
		switch (key) {
			case AK_Space: paused_ ^= 1; break;
			case AK_Right: adjustTime(5000000); break;
			case AK_Left: adjustTime(-5000000); break;
			case AK_Esc: aAppTerminate(0);
			default: break;
		}
	}

	void pointer(int dx, int dy, unsigned buttons, unsigned btn_ch) {
		if (buttons) {
			mouse.x += dx;
			mouse.y += dy;
		}

		if (btn_ch & AB_WheelUp) mouse.z += 1.f;
		if (btn_ch & AB_WheelDown) mouse.z -= 1.f;
	}
};
