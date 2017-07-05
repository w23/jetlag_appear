#include "video.h"
#include "Automation.h"
#include "fileres.h"

#include "atto/app.h"
#define ATTO_GL_H_IMPLEMENT
#include "atto/gl.h"

#define GL(F) AGL__CALL(gl##F)

static const char fs_vtx_source[] =
	"void main() {\n"
		"gl_Position = gl_Vertex;\n"
	"}"
;

typedef struct {
	GLuint fb;
	GLuint program;
	VolatileResource *fragment_source;
} RenderPass;

void passInit(RenderPass *pass, int ntex, const GLuint *tex, const char *src_file) {
	if (ntex > 0) {
		GL(GenFramebuffers(1, &pass->fb));
		GL(BindFramebuffer(GL_FRAMEBUFFER, pass->fb)); 
		for (int i = 0; i < ntex; ++i)
			GL(FramebufferTexture2D(GL_FRAMEBUFFER,
					GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, tex[i], 0));
		int status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		ATTO_ASSERT(status == GL_FRAMEBUFFER_COMPLETE);
	} else
		pass->fb = 0;

	pass->program = 0;
	pass->fragment_source = resourceOpenFile(src_file);
}

int passCheckAndUpdateProgram(RenderPass *pass) {
	if (!pass->fragment_source->updated)
		return 0;

	AGLProgram new_program = aGLProgramCreateSimple(fs_vtx_source,
			pass->fragment_source->bytes);

	if (new_program < 0) {
		aAppDebugPrintf("shader error: %s\n", a_gl_error);
		return 0;
	}

	if (pass->program > 0)
		aGLProgramDestroy(pass->program);

	pass->program = new_program;
	return 1;
}

#define MAX_RENDER_PASSES 4
#define MAX_TEXTURES ((MAX_RENDER_PASSES) * 4)

enum {
	Pass_Raymarch,
	Pass_MergeAndPost,
	Pass_ToolOut,
	Pass_MAX
};

enum {
	Texture_Noise,
	Texture_RaymarchPrimary,
	//Texture_RaymarchSecondary,
	Texture_Frame,
	Texture_MAX
};

static struct {
	int width, height;
	GLuint textures[Texture_MAX];
	RenderPass pass[Pass_MAX];
	/* TODO: rand seed ctrl; rand tex */
	GLint texture_unit[Texture_MAX];
} g;

static void drawPass(const Frame *frame, GLuint prog, GLuint fb) {
	GL(UseProgram(prog));
	GL(BindFramebuffer(GL_FRAMEBUFFER, fb));
	if (fb > 0) {
		GL(Viewport(0, 0, g.width, g.height));
		//GL(Uniform3f(glGetUniformLocation(prog, "V"), g.width, g.height, 0));
		frame->signal[0] = g.width;
		frame->signal[1] = g.height;
	} else {
		GL(Viewport(0, 0, a_app_state->width, a_app_state->height));
		//GL(Uniform3f(glGetUniformLocation(prog, "V"), a_app_state->width, a_app_state->height, 0));
		frame->signal[0] = a_app_state->width;
		frame->signal[1] = a_app_state->height;
	}
	GL(Uniform1iv(glGetUniformLocation(prog, "S"), Texture_MAX, g.texture_unit));
	GL(Uniform1fv(glGetUniformLocation(prog, "F"), frame->end - frame->start, frame->signal));
	//GL(Uniform3f(glGetUniformLocation(prog, "C"), frame->signal[0], frame->signal[1], frame->signal[2]));
	//GL(Uniform3f(glGetUniformLocation(prog, "A"), frame->signal[3], frame->signal[4], frame->signal[5]));
	//GL(Uniform3f(glGetUniformLocation(prog, "D"), frame->signal[6], frame->signal[7], frame->signal[8]));
	//GL(Uniform4f(glGetUniformLocation(prog, "M"), midi_[0], midi_[1], midi_[2], midi_[3]));

	GL(Rects(-1,-1,1,1));
}

#define NOISE_SIZE 256
static uint32_t noise_bytes[NOISE_SIZE * NOISE_SIZE];

void videoInit(int width, int height) {
	g.width = width;
	g.height = height;

	GL(GenTextures(MAX_TEXTURES, g.textures));

	{
		uint64_t seed = 1;
		for (int i = 0; i < NOISE_SIZE * NOISE_SIZE; ++i) {
			seed = 1442695040888963407ull + seed * 6364136223846793005ull;
			noise_bytes[i] = (seed >> 32);
		}

		g.texture_unit[0] = 0;
		GL(ActiveTexture(GL_TEXTURE0));
		GL(BindTexture(GL_TEXTURE_2D, g.textures[0]));
		GL(TexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, NOISE_SIZE, NOISE_SIZE, 0, GL_RGBA, GL_UNSIGNED_BYTE, noise_bytes));
		GL(TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
		GL(TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
		GL(TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
		GL(TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
	}

	for (int i = 1; i < MAX_TEXTURES; ++i) {
		g.texture_unit[i] = i;
		GL(ActiveTexture(GL_TEXTURE0 + i));
		GL(BindTexture(GL_TEXTURE_2D, g.textures[i]));
		GL(TexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, 0));
		GL(TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
		GL(TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
		GL(TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER));
		GL(TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER));
	}

	{
		GLuint *tex = g.textures + 1;
#define PASS(PASS, NTEX, FILENAME) \
	passInit(g.pass + PASS, NTEX, tex, FILENAME); tex += NTEX
		PASS(Pass_Raymarch, 1, "raymarch.glsl");
		PASS(Pass_MergeAndPost, 1, "post.glsl");
		PASS(Pass_ToolOut, 0, "out.glsl");
#undef PASS
	}
}

void videoPaint(const Frame *frame, int force_redraw) {
	int need_redraw = force_redraw;

	for (int i = 0; i < Pass_MAX; ++i)
		need_redraw |= passCheckAndUpdateProgram(g.pass + i);

	if (need_redraw) {
		for (int i = 0; i < Pass_MAX; ++i)
			drawPass(frame, g.pass[i].program, g.pass[i].fb);
	}
}

