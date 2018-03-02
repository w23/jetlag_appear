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

#define RENDER_MAX_TEXTURES 16
#define RENDER_MAX_SOURCES 16
#define RENDER_MAX_PROGRAMS 16
#define RENDER_MAX_PASSES 8

typedef struct {
	const char *name;
	GLuint glname;
	int width, height;
} RenderTexture;

typedef struct {
	const char *name;
	VolatileResource *file;
} RenderSource;

typedef struct {
	const char *name;
#define RENDER_MAX_PROGRAM_SOURCES 4
	int source_index[RENDER_MAX_PROGRAM_SOURCES];
	AGLProgram program;
} RenderProgram;

typedef struct {
	int program_index;
	GLuint fb;
	int targets;
} RenderPass;

#define DECLARE_TABLE(type,name,max) \
	type name[max]; \
	int name##s

typedef struct {
	int width, height;

	DECLARE_TABLE(RenderTexture, texture, RENDER_MAX_TEXTURES);
	DECLARE_TABLE(RenderSource, source, RENDER_MAX_SOURCES);
	DECLARE_TABLE(RenderProgram, program, RENDER_MAX_PROGRAMS);
	DECLARE_TABLE(RenderPass, pass, RENDER_MAX_PASSES);
} RenderScene;

enum {
	RenderLine_Texture,
	RenderLine_TextureNoise,
	RenderLine_PassFullscreen,
};

static void renderInitTexture(GLuint tex, int w, int h, int comp, int type, void *data, int wrap) {
	GL(BindTexture(GL_TEXTURE_2D, tex));
	GL(TexImage2D(GL_TEXTURE_2D, 0, comp, w, h, 0, GL_RGBA, type, data));
	GL(TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	GL(TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	GL(TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap));
	GL(TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap));
}

static char *strMakeCopy(const char *str) {
	const int len = strlen(str);
	char *buf = malloc(len + 1);
	memcpy(buf, str, len + 1);
	return buf;
}

static int renderParseAddTexture(const ParserCallbackParams *params) {
	RenderScene *scene = (RenderScene*)params->userdata;
	if (scene->textures == RENDER_MAX_TEXTURES) {
		MSG("Too many textures, limit: %d", scene->textures);
		return -1;
	}

	RenderTexture *tex = scene->texture + scene->textures;
	tex->name = strMakeCopy(params->args[0].s);

	glGenTextures(1, &tex->glname);
	glActiveTexture(GL_TEXTURE0 + scene->textures);
	++scene->textures;

	if (params->line_param0 == RenderLine_TextureNoise) {
		const int size = params->args[1].value.i;
		tex->width = tex->height = size;

		const int bytes = size * size * 4;
		uint8_t *buffer = malloc(bytes);
		uint32_t seed = params->args[2].value.i;
		for (int i = 0; i < bytes; ++i) {
			//seed = 1442695040888963407ull + seed * 6364136223846793005ull;
			seed = 1013904223ul + seed * 1664525ul;
			buffer[i] = (seed >> 18);
		}

		renderInitTexture(tex->glname, size, size, GL_RGBA, GL_UNSIGNED_BYTE, buffer, GL_REPEAT);
		free(buffer);
	} else {
		if (strcmp(params->args[1].s, "rgba16f") != 0) {
			MSG("Invalid format %s, only rgba16f is supported", params->args[1].s);
			return -1;
		}

		const int scale = (params->num_args > 4) ? params->args[4].value.i : 1;

		tex->width = (params->args[2].type == PAF_Var) ? scene->width / scale : params->args[2].value.i;
		tex->height = (params->args[3].type == PAF_Var) ? scene->height / scale : params->args[3].value.i;

		renderInitTexture(tex->glname, tex->width, tex->height, GL_RGBA16F,
				GL_FLOAT, 0, GL_CLAMP_TO_BORDER);
	}

	MSG("Loaded texture %s size %dx%d at index %d gl=%d", tex->name, tex->width, tex->height,
			scene->textures - 1, tex->glname);
	return 0;
}

static void renderTextureDtor(RenderTexture *tex) {
	if (tex->name)
		free((char*)tex->name);

	if (tex->glname)
		glDeleteTextures(1, &tex->glname);
}

static int renderFindSource(RenderScene *scene, const char *name) {
	for (int j = 0; j < scene->sources; ++j) {
		if (strcmp(name, scene->source[j].name) == 0)
			return j;
	}

	if (scene->sources == RENDER_MAX_SOURCES) {
		MSG("Too many sources, limit %d", RENDER_MAX_SOURCES);
		return -1;
	}

	RenderSource *src = scene->source + scene->sources;
	src->name = strMakeCopy(name);
	src->file = resourceOpenFile(name);

	if (!src->file) {
		MSG("Cannot open resource %s", name);
		return -1;
	}

	return scene->sources++;
}

static void renderSourceDtor(RenderSource *src) {
	if (src->name)
		free((char*)src->name);

	if (src->file)
		resourceClose(src->file);
}

static void renderProgramDtor(RenderProgram *prog) {
	if (prog->name)
		free((char*)prog->name);

	aGLProgramDestroy(prog->program);
}

static int renderParseAddProgram(const ParserCallbackParams *params) {
	RenderScene *scene = (RenderScene*)params->userdata;

	if (scene->programs == RENDER_MAX_PROGRAMS) {
		MSG("Too many programs, limit %d", RENDER_MAX_PROGRAMS);
		return -1;
	}

	RenderProgram *prog = scene->program + scene->programs;
	++scene->programs;

	prog->name = strMakeCopy(params->args[0].s);

	for (int i = 0; i < RENDER_MAX_PROGRAM_SOURCES; ++i)
		prog->source_index[i] = -1;

	for (int i = 0; i < params->num_args - 1; ++i) {
		const char *src = params->args[i+1].s;

		prog->source_index[i] = renderFindSource(scene, src);
		if (prog->source_index[i] == -1) {
			MSG("Cannot open source file %s for program %s", src, prog->name);
			return -1;
		}
	}

	prog->program = 0;

	return 0;
}

static int renderProgramFind(const RenderScene *scene, const char *name) {
	for (int i = 0; i < scene->programs; ++i)
		if (strcmp(scene->program[i].name, name) == 0)
			return i;

	return -1;
}

static int renderTextureFind(const RenderScene *scene, const char *name) {
	for (int i = 0; i < scene->textures; ++i)
		if (strcmp(scene->texture[i].name, name) == 0)
			return i;

	return -1;
}

static void renderPassDtor(RenderPass *pass) {
	if (pass->fb)
		glDeleteFramebuffers(1, &pass->fb);
}

static int renderParseAddPass(const ParserCallbackParams *params) {
	RenderScene *scene = (RenderScene*)params->userdata;

	if (scene->passs == RENDER_MAX_PASSES) {
		MSG("Too many passes, limit %d", RENDER_MAX_PASSES);
		return -1;
	}

	RenderPass *pass = scene->pass + scene->passs;
	pass->program_index = renderProgramFind(scene, params->args[0].s);
	if (pass->program_index == -1) {
		MSG("Cannot find program %s", params->args[0].s);
		return -1;
	}

	pass->targets = params->num_args - 1;
	pass->fb = 0;
	if (pass->targets > 0) {
		glGenFramebuffers(1, &pass->fb);
		GL(BindFramebuffer(GL_FRAMEBUFFER, pass->fb));
	}

	for (int i = 0; i < pass->targets; ++i) {
		const int tex_index = renderTextureFind(scene, params->args[i+1].s);
		if (tex_index < 0) {
			MSG("Cannot find texture %s", params->args[i+1].s);
			return -1;
		}

		const RenderTexture *tex = scene->texture + tex_index;
		GL(FramebufferTexture2D(GL_FRAMEBUFFER,
				GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, tex->glname, 0));
	}

	const int status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	ATTO_ASSERT(status == GL_FRAMEBUFFER_COMPLETE);

	++scene->passs;
	return 0;
}

static const ParserLine render_line_table[] = {
	{ "texture", RenderLine_Texture, 4, 5, {PAF_String, PAF_String, PAF_Int|PAF_Var, PAF_Int|PAF_Var, PAF_Int}, renderParseAddTexture},
	{ "texture_noise", RenderLine_TextureNoise, 3, 3, {PAF_String, PAF_Int|PAF_Var, PAF_Int}, renderParseAddTexture},
	{ "program", 0, 2, 4, {PAF_String, PAF_String, PAF_String, PAF_String}, renderParseAddProgram},
	{ "pass_fs", RenderLine_PassFullscreen, 1, 4, {PAF_String, PAF_String, PAF_String, PAF_String}, renderParseAddPass},
};

static struct {
	VolatileResource *scene_source;
	RenderScene *scene;
	int width, height;
	int output_width, output_height;
} g;

static void renderSceneDtor(RenderScene *scene) {
	for (int i = 0; i < scene->textures; ++i)
		renderTextureDtor(scene->texture + i);
	for (int i = 0; i < scene->sources; ++i)
		renderSourceDtor(scene->source + i);
	for (int i = 0; i < scene->programs; ++i)
		renderProgramDtor(scene->program + i);
	for (int i = 0; i < scene->passs; ++i)
		renderPassDtor(scene->pass + i);
}

static void renderUpdateScene() {
	RenderScene *scene = malloc(sizeof(*scene));
	memset(scene, 0, sizeof(*scene));
	scene->width = g.width;
	scene->height = g.height;

	ParserTokenizer parser;
	parser.ctx.line = g.scene_source->bytes;
	parser.ctx.prev_line = 0;
	parser.ctx.line_number = 0;
	parser.line_table = render_line_table;
	parser.line_table_length = COUNTOF(render_line_table);
	parser.userdata = scene;

	for (;;) {
		const int result = parserTokenizeLine(&parser);
		if (result == Tokenize_End) {
			RenderScene *tmp = g.scene;
			g.scene = scene;
			scene = tmp;
			break;
		}

		if (result != Tokenize_Parsed) {
			MSG("Parsing failed at line %d", parser.ctx.line_number);
			break;
		}
	}

	if (scene) {
		renderSceneDtor(scene);
		free(scene);
	}
}

int videoInit(int width, int height, const char *filename) {
	aGLInit();

	g.scene_source = resourceOpenFile(filename);
	if (!g.scene_source) {
		MSG("Cannot open scene file %s", filename);
		return 0;
	}

	g.width = g.output_width = width;
	g.height = g.output_height = height;

	g.scene = 0;

	return 1;
}

void videoOutputResize(int width, int height) {
	g.output_width = width;
	g.output_height = height;
}

void renderProgramCheckUpdate(RenderProgram *prog) {
	const char *fragment[RENDER_MAX_PROGRAM_SOURCES + 1];

	int updated = 0;
	for (int i = 0; i < RENDER_MAX_PROGRAM_SOURCES; ++i) {
		if (prog->source_index[i] < 0) {
			fragment[i] = NULL;
			break;
		}

		const RenderSource *src = g.scene->source + prog->source_index[i];

		updated |= src->file->updated;
		fragment[i] = src->file->bytes;
	}

	if (!updated)
		return;

	MSG("Reloading %s", prog->name);

	const char *const vertex[] = { fs_vtx_source, NULL };
	AGLProgram new_program = aGLProgramCreate(vertex, fragment);

	if (new_program < 0) {
		MSG("shader error: %s", a_gl_error);
		return;
	}

	if (prog->program > 0)
		aGLProgramDestroy(prog->program);

	prog->program = new_program;
}

void videoPaint() {
	if (g.scene_source->updated) {
		renderUpdateScene();
		return;
	}

	if (!g.scene)
		return;

	int can_draw = 1;
	for (int i = 0; i < g.scene->programs; ++i) {
		RenderProgram *prog = g.scene->program + i;
		renderProgramCheckUpdate(prog);
		if (prog->program < 0)
			can_draw = 0;
	}

	if (!can_draw)
		return;

	// ugh
	int samplers[RENDER_MAX_TEXTURES];
	for (int i = 0; i < RENDER_MAX_TEXTURES; ++i)
		samplers[i] = i;

	for (int i = 0; i < g.scene->passs; ++i) {
		const RenderPass *pass = g.scene->pass + i;
		const RenderProgram *prog = g.scene->program + pass->program_index;

		GL(UseProgram(prog->program));
		GL(BindFramebuffer(GL_FRAMEBUFFER, pass->fb));

		if (pass->fb > 0) {
			GL(Viewport(0, 0, g.width, g.height));

			const int Vloc = glGetUniformLocation(prog->program, "VIEWPORT");
			if (Vloc >= 0) {
				const float V[] = { (float)g.width, (float)g.height };
				GL(Uniform2fv(Vloc, 1, V));
			}

			const GLuint bufs[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
			GL(DrawBuffers(pass->targets, bufs));
		} else {
			GL(Viewport(0, 0, g.output_width, g.output_height));

			int Vloc = glGetUniformLocation(prog->program, "VIEWPORT");
			if (Vloc >= 0) {
				const float V[] = { (float)g.output_width, (float)g.output_height };
				GL(Uniform2fv(Vloc, 1, V));
			}
		}

		// FIXME
		//const int itime = signals[0] * 8 * 44100;
		static int itime = 0;
		itime += 8 * 44100 / 280;

		GL(Uniform1iv(glGetUniformLocation(prog->program, "S"), g.scene->textures, samplers));
		//GL(Uniform1fv(glGetUniformLocation(pass->program, "F"), num_signals, signals));
		GL(Uniform1iv(glGetUniformLocation(prog->program, "F"), 1, &itime));
		GL(Rects(-1,-1,1,1));
	}
}

