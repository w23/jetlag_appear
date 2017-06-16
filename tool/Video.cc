#include "Video.h"
#include "Timeline.h"

#define ATTO_GL_H_IMPLEMENT
#include "atto/gl.h"

namespace {
const char fs_vtx_source[] =
	"void main() {\n"
		"gl_Position = gl_Vertex;\n"
	"}"
;

void createTexture(GLint t, int w, int h) {
	AGL__CALL(glBindTexture(GL_TEXTURE_2D, t));
	AGL__CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, w, h, 0, GL_RGBA, GL_FLOAT, 0));
	AGL__CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	AGL__CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	AGL__CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER));
	AGL__CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER));
}
} // namespace

bool Program::update() {
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

Video::Video(int width, int height)//const char *raymarch_file, const char *post_file)
	: width(width), height(height)
	, fs_vtx(fs_vtx_source)
	, raymarch_src("raymarch.glsl")
	, raymarch_prg(fs_vtx, raymarch_src)
	, post_src("post.glsl")
	, post_prg(fs_vtx, post_src)
	, out_src("out.glsl")
	, out_prg(fs_vtx, out_src)
{
	tex_[0] = fb_[0] = 0;
	AGL__CALL(glGenTextures(FbTex_COUNT-1, tex_ + 1));
	AGL__CALL(glGenFramebuffers(FbTex_COUNT-1, fb_ + 1));

	for (int i = 0; i < FbTex_COUNT; ++i)
		aAppDebugPrintf("tex_[%d] = %u; fb_[%d] = %u;", i, tex_[i], i, fb_[i]);

	createTexture(tex_[FbTex_Ray], width, height);
	AGL__CALL(glBindFramebuffer(GL_FRAMEBUFFER, fb_[FbTex_Ray]));
	AGL__CALL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex_[FbTex_Ray], 0));
	int status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	ATTO_ASSERT(status == GL_FRAMEBUFFER_COMPLETE);

	createTexture(tex_[FbTex_Frame], width, height);
	AGL__CALL(glBindFramebuffer(GL_FRAMEBUFFER, fb_[FbTex_Frame]));
	AGL__CALL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex_[FbTex_Frame], 0));
	status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	ATTO_ASSERT(status == GL_FRAMEBUFFER_COMPLETE);
}

bool Video::paint(const Timeline &timeline, float now, bool force_redraw) {
	const Timeline::Sample TV = timeline.sample(now);

	bool need_redraw = raymarch_prg.update();
	need_redraw |= post_prg.update();
	need_redraw |= out_prg.update();

	if (force_redraw || need_redraw) {
		drawPass(now, TV, 0, raymarch_prg.program(), FbTex_Ray);
		drawPass(now, TV, FbTex_Ray, post_prg.program(), FbTex_Frame);
	}
	drawPass(now, TV, FbTex_Frame, out_prg.program(), 0);

	return need_redraw;
}

void Video::drawPass(float now, const Timeline::Sample &TV, int tex, int prog, int fb) {
	AGL__CALL(glBindTexture(GL_TEXTURE_2D, tex));
	AGL__CALL(glUseProgram(prog));
	AGL__CALL(glBindFramebuffer(GL_FRAMEBUFFER, fb));
	if (fb > 0) {
		AGL__CALL(glViewport(0, 0, width, height));
		AGL__CALL(glUniform3f(glGetUniformLocation(prog, "V"), width, height, now));
	} else {
		AGL__CALL(glViewport(0, 0, a_app_state->width, a_app_state->height));
		AGL__CALL(glUniform3f(glGetUniformLocation(prog, "V"), a_app_state->width, a_app_state->height, now));
	}
	AGL__CALL(glUniform1i(glGetUniformLocation(prog, "B"), 0));
	AGL__CALL(glUniform3f(glGetUniformLocation(prog, "C"), TV[0], TV[1], TV[2]));
	AGL__CALL(glUniform3f(glGetUniformLocation(prog, "A"), TV[3], TV[4], TV[5]));
	AGL__CALL(glUniform3f(glGetUniformLocation(prog, "D"), TV[6], TV[7], TV[8]));
	//AGL__CALL(glUniform4f(glGetUniformLocation(prog, "M"), midi_[0], midi_[1], midi_[2], midi_[3]));

	AGL__CALL(glUniform1f(glGetUniformLocation(prog, "TPCT"), (float)now / 120.f));
	AGL__CALL(glRects(-1,-1,1,1));
}
