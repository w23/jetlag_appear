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
	Pass_BlurReflection2,
	Pass_Composite,
	Pass_DofTap,
	Pass_MergeAndPost,
	Pass_ToolOut,
	Pass_MAX
};

enum {
	Texture_Noise,
	Texture_RaymarchPrimary,
	Texture_RaymarchReflection,
	//Texture_RaymarchReflectionMeta,
	Texture_RaymarchReflectionBlur,
	Texture_RaymarchReflectionBlur2,
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
	if (pass->program < 1)
		return;

	GL(UseProgram(pass->program));
	GL(BindFramebuffer(GL_FRAMEBUFFER, pass->fb));
	if (pass->fb > 0) {
		GL(Viewport(0, 0, g.width, g.height));
		int Vloc = glGetUniformLocation(pass->program, "VIEWPORT");
		if (Vloc >= 0)
			GL(Uniform2f(Vloc, (float)g.width, (float)g.height));
		const GLuint bufs[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
		GL(DrawBuffers(pass->targets, bufs));
	} else {
		GL(Viewport(0, 0, g.output_width, g.output_height));
		int Vloc = glGetUniformLocation(pass->program, "VIEWPORT");
		if (Vloc >= 0)
			GL(Uniform2f(Vloc, (float)g.output_width, (float)g.output_height));
	}
	GL(Uniform1iv(glGetUniformLocation(pass->program, "S"), Texture_MAX, g.texture_unit));
	GL(Uniform1fv(glGetUniformLocation(pass->program, "F"), num_signals, signals));
	GL(Rects(-1,-1,1,1));
}

#define NOISE_SIZE 256
static uint8_t noise_bytes[NOISE_SIZE * NOISE_SIZE * 4];

#define MAX_GRAPH_SAMPLES 1024
static struct {
	AGLAttribute attribs[2];
	AGLProgramUniform uniforms[4];
	AGLDrawSource source;
	float x[MAX_GRAPH_SAMPLES];
} graph;

static void graphInit() {
	for (int i = 0; i < MAX_GRAPH_SAMPLES; ++i)
		graph.x[i] = (float)i;
	graph.attribs[0].name = "x";
	graph.attribs[0].buffer = 0;
	graph.attribs[0].size = 1;
	graph.attribs[0].type = GL_FLOAT;
	graph.attribs[0].normalized = 0;
	graph.attribs[0].stride = 0;
	graph.attribs[0].ptr = graph.x;
	graph.attribs[1].name = "y";
	graph.attribs[1].buffer = 0;
	graph.attribs[1].size = 1;
	graph.attribs[1].type = GL_FLOAT;
	graph.attribs[1].normalized = 0;
	graph.uniforms[0].name = "x_off";
	graph.uniforms[0].type = AGLAT_Float;
	graph.uniforms[0].count = 1;
	graph.uniforms[1].name = "x_scale";
	graph.uniforms[1].type = AGLAT_Float;
	graph.uniforms[1].count = 1;
	graph.uniforms[2].name = "y_off";
	graph.uniforms[2].type = AGLAT_Float;
	graph.uniforms[2].count = 1;
	graph.uniforms[3].name = "y_scale";
	graph.uniforms[3].type = AGLAT_Float;
	graph.uniforms[3].count = 1;
	graph.source.primitive.first = 0;
	graph.source.primitive.mode = GL_LINE_STRIP;
	graph.source.primitive.cull_mode = AGLCM_Disable;
	graph.source.primitive.index.buffer = 0;
	graph.source.primitive.index.data.ptr = 0;
	graph.source.primitive.index.type = 0;
	graph.source.primitive.front_face = AGLFF_CounterClockwise;
	graph.source.attribs.p = graph.attribs;
	graph.source.attribs.n = COUNTOF(graph.attribs);
	graph.source.uniforms.p = graph.uniforms;
	graph.source.uniforms.n = COUNTOF(graph.uniforms);
	graph.source.program = aGLProgramCreateSimple(
		"uniform float x_off, x_scale, y_off, y_scale;\n"
		"attribute float x, y;\n"
		"void main() {\n"
			"gl_Position= vec4(x * x_scale + x_off, y * y_scale + y_off, 0, 1.);\n"
		"}",
		"void main() { gl_FragColor = vec4(1.); }");
	if (graph.source.program < 0) {
		MSG("graph program compile failed: %s", a_gl_error);
		abort();
	}
	aGLUniformLocate(graph.source.program, graph.uniforms, COUNTOF(graph.uniforms));
	aGLAttributeLocate(graph.source.program, graph.attribs, COUNTOF(graph.attribs));
}

void videoInit(int width, int height) {
	aGLInit();
	graphInit();

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

		g.texture_unit[0] = 0;
		GL(ActiveTexture(GL_TEXTURE0));
		GL(BindTexture(GL_TEXTURE_2D, g.textures[0]));
		GL(TexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, NOISE_SIZE, NOISE_SIZE, 0, GL_RGBA, GL_UNSIGNED_BYTE, noise_bytes));
		GL(TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
		GL(TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
		GL(TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
		GL(TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
	}

	for (int i = 1; i < Texture_MAX; ++i) {
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
		PASS(Pass_Raymarch, 2, "raymarch.glsl");
		PASS(Pass_BlurReflection, 1, "blur_reflection.glsl");
		PASS(Pass_BlurReflection2, 1, "blur_reflection2.glsl");
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

static void paintGraph(const float *values, int count, float x_off, float x_scale, float y_off, float y_scale) {
	graph.attribs[1].stride = 0;
	graph.attribs[1].ptr = values;
	graph.source.primitive.count = count;
	graph.uniforms[0].value.pf = &x_off;
	graph.uniforms[1].value.pf = &x_scale;
	graph.uniforms[2].value.pf = &y_off;
	graph.uniforms[3].value.pf = &y_scale;

	AGLDrawMerge merge;
	merge.depth.mode = AGLDM_Disabled;
	merge.blend.enable = 0;

	AGLDrawTarget target;
	target.framebuffer = 0;
	target.viewport.x = target.viewport.y = 0;
	target.viewport.w = g.output_width;
	target.viewport.h = g.output_height;

	aGLDraw(&graph.source, &merge, &target);
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

	if (0) {
	const int samples = 2000;
	const int diag_signals_count = 4;//MAX_DIAG_SIGNALS;
	const float h = 2.f / diag_signals_count;
	const float kx = 2.f / (samples - 1);
	MEMORY_BARRIER();
	const int pos = diag_signals.writepos;
	for (int i = 0; i < diag_signals_count; ++i) {
		const float y_off = 1.f - h * i - h * .5f;
		for (int j = 0; j < samples;) {
			const int left = samples - j;
			const int begin = (pos + MAX_DIAG_SAMPLES - left) % MAX_DIAG_SAMPLES;
			const int end = begin + left < MAX_DIAG_SAMPLES ? begin + left : MAX_DIAG_SAMPLES;
			const int length = end - begin > MAX_GRAPH_SAMPLES ? MAX_GRAPH_SAMPLES : end - begin;

			paintGraph(diag_signals.signal[i].f + begin, length, -1.f + j * kx, kx, y_off, h * .5);
			j += length;
		}
	}
	}
}

