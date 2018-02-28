#include "common.h"

#include "atto/app.h"
#define ATTO_GL_DEBUG
#define ATTO_GL_H_IMPLEMENT
#include "atto/gl.h"

#define GL(F) AGL__CALL(gl##F)

static const char fs_vtx_source[] =
	"void main() {\n"
		"gl_Position = gl_Vertex;\n"
	"}"
;

typedef struct {
	const char *name;
	GLuint fb;
	GLuint program;
	int targets;
	VolatileResource *fragment_source;
} RenderPass;

enum {
	Pass_Raymarch,
	Pass_BlurReflection,
	Pass_Composite,
	Pass_DofTap,
	Pass_MergeAndPost,
	Pass_ToolOut,
	Pass_MAX
};

enum {
	Texture_Noise,
	//Texture_Data,
	Texture_RaymarchPrimary,
	Texture_RaymarchReflection,
	Texture_RaymarchReflectionBlur,
	Texture_RaymarchCombined,
	Texture_DofTapNear,
	Texture_DofTapFar,
	Texture_Frame,
	Texture_MAX
};

static struct {
	int width, height;
	int output_width, output_height;
	GLuint textures[Texture_MAX];
	RenderPass pass[Pass_MAX];
	GLint texture_unit[Texture_MAX];
	VolatileResource *common_header;
} g;

void passInit(RenderPass *pass, int ntex, const GLuint *tex, const char *src_file) {
	if (ntex > 0) {
		pass->targets = ntex;
		GL(GenFramebuffers(1, &pass->fb));
		GL(BindFramebuffer(GL_FRAMEBUFFER, pass->fb));
		for (int i = 0; i < ntex; ++i) {
			MSG("%s %p %d %d %d", src_file, pass, pass->fb, i, tex[i]);
			GL(FramebufferTexture2D(GL_FRAMEBUFFER,
					GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, tex[i], 0));
		}
		int status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		ATTO_ASSERT(status == GL_FRAMEBUFFER_COMPLETE);
	} else
		pass->fb = 0;

	pass->name = src_file;

	pass->program = 0;
	pass->fragment_source = resourceOpenFile(src_file);
}

static /*__forceinline*/ void initTexture(GLuint tex, int w, int h, int comp, int type, void *data, int wrap) {
	GL(BindTexture(GL_TEXTURE_2D, tex));
	GL(TexImage2D(GL_TEXTURE_2D, 0, comp, w, h, 0, GL_RGBA, type, data));
	GL(TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	GL(TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	GL(TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap));
	GL(TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap));
}

int passCheckAndUpdateProgram(RenderPass *pass) {
	if (!pass->fragment_source->updated && !g.common_header->updated)
		return 0;

	MSG("Loading for %s", pass->name);

	const char * const vertex[] = { fs_vtx_source, NULL };
	const char * const fragment[] = {
		g.common_header->bytes,
		pass->fragment_source->bytes,
		NULL };
	AGLProgram new_program = aGLProgramCreate(vertex, fragment);

	if (new_program < 0) {
		MSG("shader error: %s", a_gl_error);
		return 0;
	}

	if (pass->program > 0)
		aGLProgramDestroy(pass->program);

	pass->program = new_program;
	return 1;
}

static void drawPass(float *signals, int num_signals, const RenderPass *pass) {
	(void)num_signals;
	if (pass->program < 1)
		return;

	GL(UseProgram(pass->program));
	GL(BindFramebuffer(GL_FRAMEBUFFER, pass->fb));
	if (pass->fb > 0) {
		GL(Viewport(0, 0, g.width, g.height));
		int Vloc = glGetUniformLocation(pass->program, "VIEWPORT");
		const float V[] = { (float)g.width, (float)g.height };
		if (Vloc >= 0)
			GL(Uniform2fv(Vloc, 1, V));
		const GLuint bufs[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
		GL(DrawBuffers(pass->targets, bufs));
	} else {
		GL(Viewport(0, 0, g.output_width, g.output_height));
		int Vloc = glGetUniformLocation(pass->program, "VIEWPORT");
		const float V[] = { (float)g.output_width, (float)g.output_height };
		if (Vloc >= 0)
			GL(Uniform2fv(Vloc, 1, V));
	}
	const int itime = signals[0] * 8 * 44100;

	GL(Uniform1iv(glGetUniformLocation(pass->program, "S"), Texture_MAX, g.texture_unit));
	//GL(Uniform1fv(glGetUniformLocation(pass->program, "F"), num_signals, signals));
	GL(Uniform1iv(glGetUniformLocation(pass->program, "F"), 1, &itime));
	GL(Rects(-1,-1,1,1));
}

#define NOISE_SIZE 256
static uint8_t noise_bytes[NOISE_SIZE * NOISE_SIZE * 4];

void videoInit(int width, int height) {
	aGLInit();

	g.width = width;
	g.height = height;

	g.common_header = resourceOpenFile("header.glsl");

	GL(GenTextures(Texture_MAX, g.textures));

	{
		uint32_t seed = 0;
		for (int i = 0; i < NOISE_SIZE * NOISE_SIZE * 4; ++i) {
			//seed = 1442695040888963407ull + seed * 6364136223846793005ull;
			seed = 1013904223ul + seed * 1664525ul;
			noise_bytes[i] = (seed >> 18);
		}

		for (int i = 0; i < Texture_MAX; ++i)
			g.texture_unit[i] = i;

		glActiveTexture(GL_TEXTURE0);
		initTexture(g.textures[Texture_Noise], NOISE_SIZE, NOISE_SIZE, GL_RGBA, GL_UNSIGNED_BYTE, noise_bytes, GL_REPEAT);
		glActiveTexture(GL_TEXTURE1);
		//initTexture(g.textures[Texture_Data], 256, 32, GL_RGBA, GL_UNSIGNED_BYTE, 0, GL_CLAMP_TO_BORDER);
		//glActiveTexture(GL_TEXTURE2);
		initTexture(g.textures[Texture_RaymarchPrimary], g.width, g.height, GL_RGBA16F, GL_FLOAT, 0, GL_CLAMP_TO_BORDER);
		glActiveTexture(GL_TEXTURE2);
		initTexture(g.textures[Texture_RaymarchReflection], g.width, g.height, GL_RGBA16F, GL_FLOAT, 0, GL_CLAMP_TO_BORDER);
		glActiveTexture(GL_TEXTURE3);
		initTexture(g.textures[Texture_RaymarchReflectionBlur], g.width/2, g.height/2, GL_RGBA16F, GL_FLOAT, 0, GL_CLAMP_TO_BORDER);
		glActiveTexture(GL_TEXTURE4);
		initTexture(g.textures[Texture_RaymarchCombined], g.width, g.height, GL_RGBA16F, GL_FLOAT, 0, GL_CLAMP_TO_BORDER);
		glActiveTexture(GL_TEXTURE5);
		initTexture(g.textures[Texture_DofTapNear], g.width, g.height, GL_RGBA16F, GL_FLOAT, 0, GL_CLAMP_TO_BORDER);
		glActiveTexture(GL_TEXTURE6);
		initTexture(g.textures[Texture_DofTapFar], g.width, g.height, GL_RGBA16F, GL_FLOAT, 0, GL_CLAMP_TO_BORDER);
		glActiveTexture(GL_TEXTURE7);
		initTexture(g.textures[Texture_Frame], g.width, g.height, GL_RGBA16F, GL_FLOAT, 0, GL_CLAMP_TO_BORDER);
	}

	{
		GLuint *tex = g.textures + Texture_RaymarchPrimary;
#define PASS(PASS, NTEX, FILENAME) \
	passInit(g.pass + PASS, NTEX, tex, FILENAME); tex += NTEX
		PASS(Pass_Raymarch, 2, "raymarch.glsl");
		PASS(Pass_BlurReflection, 1, "blur_reflection.glsl");
		PASS(Pass_Composite, 1, "composite.glsl");
		PASS(Pass_DofTap, 2, "dof_tap.glsl");
		PASS(Pass_MergeAndPost, 1, "post.glsl");
		PASS(Pass_ToolOut, 0, "out.glsl");
#undef PASS
	}
}

void videoOutputResize(int width, int height) {
	g.output_width = width;
	g.output_height = height;
}

void videoPaint(float *signals, int num_signals, int force_redraw) {
	int need_redraw = force_redraw;

	for (int i = 0; i < Pass_MAX; ++i)
		need_redraw |= passCheckAndUpdateProgram(g.pass + i);

	if (need_redraw)
		for (int i = 0; i < Pass_MAX; ++i)
			drawPass(signals, num_signals, g.pass + i);
	else
			drawPass(signals, num_signals, g.pass + Pass_MAX - 1);
}

