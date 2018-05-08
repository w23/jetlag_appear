#include "common.h"

#include "atto/app.h"
#define ATTO_GL_DEBUG
#define ATTO_GL_H_IMPLEMENT
#include "atto/gl.h"

#include <ctype.h>
#include <stdio.h>

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

#define RENDER_MAX_SOURCE_VARIABLES 32

typedef struct {
	int var_index;
	int offset, length;
} RenderSourceVarLoc;

typedef struct {
	const char *name;
	VolatileResource *source;
	int source_sequence;

	int vars;
	VarDesc var[RENDER_MAX_SOURCE_VARIABLES];

	int var_locs;
	RenderSourceVarLoc loc[RENDER_MAX_SOURCE_VARIABLES];

	const char *processed_source;
} RenderSource;

typedef struct {
	const char *name;
#define RENDER_MAX_PROGRAM_SOURCES 4
	int source_index[RENDER_MAX_PROGRAM_SOURCES];
	const char *header;
	AGLProgram program;
} RenderProgram;

typedef struct {
	int blend;
	int program_index;
	GLuint fb;
	int targets;
	struct {
		int frames;
		float time;
	} stats;
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

typedef struct {
	RenderScene *scene;
	int blend_enabled;
} RenderParseContext;

enum {
	RenderLine_Texture,
	RenderLine_TextureNoise,
	RenderLine_PassFullscreen,
	RenderLine_Blend,
};

static void renderInitTexture(GLuint tex, int w, int h, int comp, int type, void *data, int wrap) {
	GL(BindTexture(GL_TEXTURE_2D, tex));
	GL(TexImage2D(GL_TEXTURE_2D, 0, comp, w, h, 0, GL_RGBA, type, data));
	GL(TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	GL(TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	GL(TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap));
	GL(TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap));
}

char *strMakeCopy(const char *str) {
	const int len = (int)strlen(str);
	char *buf = malloc(len + 1);
	memcpy(buf, str, len + 1);
	return buf;
}

static int renderParseAddTexture(const ParserCallbackParams *params) {
	RenderParseContext * const ctx = (RenderParseContext*)params->userdata;
	RenderScene * const scene = ctx->scene;
	if (scene->textures == RENDER_MAX_TEXTURES) {
		MSG("Too many textures, limit: %d", scene->textures);
		return -1;
	}

	RenderTexture *tex = scene->texture + scene->textures;
	tex->name = strMakeCopy(params->args[0].s);

	GL(GenTextures(1, &tex->glname));
	GL(ActiveTexture(GL_TEXTURE0 + scene->textures));
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
		GL(DeleteTextures(1, &tex->glname));
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
	src->processed_source = NULL;
	src->name = strMakeCopy(name);
	src->source = resourceOpenFile(name);
	src->source_sequence = 0;
	src->vars = 0;
	src->var_locs = 0;

	if (!src->source) {
		MSG("Cannot open resource %s", name);
		return -1;
	}

	return scene->sources++;
}

static void renderSourceDtor(RenderSource *src) {
	if (src->name)
		free((char*)src->name);

	if (src->source)
		resourceClose(src->source);

	if (src->processed_source)
		free((char*)src->processed_source);
}

static void renderProgramDtor(RenderProgram *prog) {
	if (prog->name)
		free((char*)prog->name);

	if (prog->header)
		free((char*)prog->header);

	aGLProgramDestroy(prog->program);
}

static int renderParseAddProgram(const ParserCallbackParams *params) {
	RenderParseContext * const ctx = (RenderParseContext*)params->userdata;
	RenderScene * const scene = ctx->scene;

	if (scene->programs == RENDER_MAX_PROGRAMS) {
		MSG("Too many programs, limit %d", RENDER_MAX_PROGRAMS);
		return -1;
	}

	RenderProgram *prog = scene->program + scene->programs;
	++scene->programs;

	prog->name = strMakeCopy(params->args[0].s);
	prog->header = NULL;

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

	prog->program = -1;

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
		GL(DeleteFramebuffers(1, &pass->fb));
}

static int renderParseAddPass(const ParserCallbackParams *params) {
	RenderParseContext * const ctx = (RenderParseContext*)params->userdata;
	RenderScene * const scene = ctx->scene;

	if (scene->passs == RENDER_MAX_PASSES) {
		MSG("Too many passes, limit %d", RENDER_MAX_PASSES);
		return -1;
	}

	RenderPass *pass = scene->pass + scene->passs;
	pass->blend = ctx->blend_enabled;
	pass->program_index = renderProgramFind(scene, params->args[0].s);
	if (pass->program_index == -1) {
		MSG("Cannot find program %s", params->args[0].s);
		return -1;
	}

	pass->targets = params->num_args - 1;
	pass->fb = 0;
	if (pass->targets > 0) {
		GL(GenFramebuffers(1, &pass->fb));
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

static int renderParseSetBlend(const ParserCallbackParams *params) {
	RenderParseContext * const ctx = (RenderParseContext*)params->userdata;

	ctx->blend_enabled = !!params->args[0].value.i;
	return 0;
}

static const ParserLine render_line_table[] = {
	{ "texture", RenderLine_Texture, 4, 5, {PAF_String, PAF_String, PAF_Int|PAF_Var, PAF_Int|PAF_Var, PAF_Int}, renderParseAddTexture},
	{ "texture_noise", RenderLine_TextureNoise, 3, 3, {PAF_String, PAF_Int|PAF_Var, PAF_Int}, renderParseAddTexture},
	{ "program", 0, 2, 4, {PAF_String, PAF_String, PAF_String, PAF_String}, renderParseAddProgram},
	{ "pass_fs", RenderLine_PassFullscreen, 1, 4, {PAF_String, PAF_String, PAF_String, PAF_String}, renderParseAddPass},
	{ "blend", RenderLine_Blend, 1, 1, {PAF_Int}, renderParseSetBlend},
};

static struct {
	VolatileResource *scene_source;
	RenderScene *scene;
	int width, height;

	struct {
		VolatileResource *program_source;
		int program_sequence;
		AGLProgram program;
		int width, height;
		GLuint tex, fb;
	} output;
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

	RenderParseContext ctx;
	ctx.scene = scene;
	ctx.blend_enabled = 0;

	ParserTokenizer parser;
	parser.ctx.line = g.scene_source->bytes;
	parser.ctx.prev_line = 0;
	parser.ctx.line_number = 0;
	parser.line_table = render_line_table;
	parser.line_table_length = COUNTOF(render_line_table);
	parser.userdata = &ctx;

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

	g.width = g.output.width = width;
	g.height = g.output.height = height;

	g.scene = 0;

	GL(GenTextures(1, &g.output.tex));
	GL(ActiveTexture(GL_TEXTURE0 + RENDER_MAX_TEXTURES));
	renderInitTexture(g.output.tex, g.width, g.height, GL_RGBA, GL_UNSIGNED_BYTE, NULL, GL_CLAMP_TO_BORDER);

	GL(GenFramebuffers(1, &g.output.fb));
	GL(BindFramebuffer(GL_FRAMEBUFFER, g.output.fb));
	GL(FramebufferTexture2D(GL_FRAMEBUFFER,
			GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, g.output.tex, 0));
	const int status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	ATTO_ASSERT(status == GL_FRAMEBUFFER_COMPLETE);

	g.output.program_sequence = 0;
	g.output.program = 0;
	g.output.program_source = resourceOpenFile("out.glsl");
	if (!g.output.program_source) {
		MSG("Cannot open out.glsl");
		return 0;
	}

	return 1;
}

void videoOutputResize(int width, int height) {
	g.output.width = width;
	g.output.height = height;
}

static int renderReloadProgram(AGLProgram *prog, const char * const *vertex, const char * const *fragment) {
	AGLProgram new_program = aGLProgramCreate(vertex, fragment);

	if (new_program < 1) {
		MSG("shader error: %s", a_gl_error);
		return 0;
	}

	if (*prog > 0)
		aGLProgramDestroy(*prog);

	*prog = new_program;
	MSG("New program: %d", new_program);
	return 1;
}

static int sourceCheckUpdate(RenderSource *src) {
	if (!src->source->updated || src->source->sequence == src->source_sequence)
		return 0;

	if (src->processed_source) {
		free((char*)src->processed_source);
		src->processed_source = NULL;
	}

	src->vars = 0;
	src->var_locs = 0;
	const char * const str_begin = src->source->bytes;
	const char *str = src->source->bytes;

	MutableString processed;
	mutableStringInit(&processed);
	for (;;) {
		const char *var = str, *end = str;
		while (*var && !(var[0] == '$' && var[1] == '(')) ++var;

		mutableStringAppend(&processed, str, (int)(var - str));

		if (!*var)
			// no further variables found
			break;

		const char *vtype = var + 2;
		while (*vtype && isspace(*vtype)) ++vtype;
		if (!*vtype) {
			end = vtype;
			goto malformed;
		}

		const char *vtype_end = vtype;
		while (*vtype_end && (isalnum(*vtype_end) || *vtype_end == '_')) ++vtype_end;
		if (!*vtype_end) {
			end = vtype_end;
			goto malformed;
		}

		const char *vname = vtype_end + 1;
		while (*vname && isspace(*vname)) ++vname;
		if (!*vname) {
			end = vname;
			goto malformed;
		}

		const char *vname_end = vname;
		while (*vname_end && (isalnum(*vname_end) || *vname_end == '_')) ++vname_end;
		if (!*vname_end) {
			end = vname_end;
			goto malformed;
		}

		const char *vend = vname_end;
		while (*vend && isspace(*vend)) ++vend;
		end = vend;
		if (*vend != ')')
			goto malformed;

		VarDesc new_var;
		new_var.type = varGetType(stringView(vtype, (int)(vtype_end - vtype)));
		if (new_var.type == VarType_None) {
			MSG("Invalid variable type %.*s", vtype_end - vtype, vtype);
			goto malformed;
		}

		const int vname_length = (int)(vname_end - vname);
		if (vname_length > MAX_VARIABLE_NAME_LENGTH) {
			MSG("VarDesc name %.*s (%d) is too long, limit %d", vname_length, vname, MAX_VARIABLE_NAME_LENGTH);
			goto malformed;
		}

		memcpy(new_var.name, vname, vname_length);
		new_var.name[vname_length] = '\0';

		int var_index = -1;
		for (int j = 0; j < src->vars; ++j) {
			const VarDesc *v = src->var + j;
			if (strcmp(v->name, new_var.name) == 0) {
				if (v->type == new_var.type) {
					var_index = j;
					break;
				} else {
					MSG("Variable %s (type %s) was already defined with different type: %s",
						new_var.name, varGetTypeName(new_var.type), varGetTypeName(v->type));
					goto malformed;
				}
			}
		}

		if (var_index < 0) {
			if (src->vars >= RENDER_MAX_SOURCE_VARIABLES) {
				MSG("Too many variables, limit %d", RENDER_MAX_SOURCE_VARIABLES);
				goto malformed;
			}

			memcpy(src->var + src->vars, &new_var, sizeof(new_var));
			var_index = src->vars;
			++src->vars;
		}

		if (src->var_locs >= RENDER_MAX_SOURCE_VARIABLES) {
			MSG("Too many variable locations, limit %d", RENDER_MAX_SOURCE_VARIABLES);
			goto malformed;
		}

		src->loc[src->var_locs].var_index = var_index;
		src->loc[src->var_locs].offset = var - str_begin;
		src->loc[src->var_locs].length = vend + 1 - var;
		++src->var_locs;

		str = vend + 1;
		mutableStringAppend(&processed, vname, vname_length);
		continue;

malformed:
		MSG("Error parsing variable %.*s", end - var, var);
		mutableStringDestroy(&processed);
		break;
	} // for (;;) search for vars

	if (processed.length > 0) {
		src->processed_source = mutableStringRelease(&processed);
		src->source_sequence = src->source->sequence;
		MSG("Source:\n%s", src->processed_source);
		return 1;
	}

	return 0;
}

static int renderSourceExport(const RenderSource *src) {
	if (!src->processed_source) {
		MSG("Source %s is not ready for export", src->name);
		return 0;
	}

	MutableString name;
	mutableStringInit(&name);
	mutableStringAppendZ(&name, src->name);
	mutableStringAppendZ(&name, ".glsl");
	FILE *f = fopen(name.str, "wb");
	if (!f) {
		MSG("Cannot open file %s for writing", name.str);
		mutableStringDestroy(&name);
		return 0;
	}
	mutableStringDestroy(&name);

	const char *source = src->source->bytes;
	int offset = 0;
	for (int i = 0; i < src->var_locs; ++i) {
		const RenderSourceVarLoc *loc = src->loc + i;
		const int segment = loc->offset - offset;
		fwrite(source + offset, 1, segment, f);
		offset = loc->offset + loc->length;

		const VarDesc *var = src->var + loc->var_index;
		AVec4f value;
		const int uniform_offset = varExportGetVarOffset(var, &value);
		if (uniform_offset < 0) {
			switch (var->type) {
			case VarType_None: /* FIXME assert */ continue;
			case VarType_Float:
#define VAR_FMT "%.3f"
				fprintf(f, VAR_FMT, value.x);
				break;
			case VarType_Vec2:
				fprintf(f, "vec2(" VAR_FMT "," VAR_FMT ")", value.x, value.y);
				break;
			case VarType_Vec3:
				fprintf(f, "vec3(" VAR_FMT "," VAR_FMT "," VAR_FMT ")", value.x, value.y, value.z);
				break;
			case VarType_Vec4:
				fprintf(f, "vec4(" VAR_FMT "," VAR_FMT "," VAR_FMT "," VAR_FMT ")", value.x, value.y, value.z, value.w);
				break;
			}
		} else {
			switch (var->type) {
			case VarType_None: /* FIXME assert */ continue;
			case VarType_Float:
				fprintf(f, "F[%d]", uniform_offset);
				break;
			case VarType_Vec2:
				fprintf(f, "vec2(F[%d],F[%d])", uniform_offset, uniform_offset + 1);
				break;
			case VarType_Vec3:
				fprintf(f, "vec3(F[%d],F[%d],F[%d])", uniform_offset, uniform_offset + 1, uniform_offset + 2);
				break;
			case VarType_Vec4:
				fprintf(f, "vec4(F[%d],F[%d],F[%d],F[%d])", uniform_offset, uniform_offset + 1, uniform_offset + 2, uniform_offset + 3);
				break;
			}
		}
	}

	if (offset < src->source->size) {
		const int segment = src->source->size - offset;
		fwrite(source + offset, 1, segment, f);
	}

	fclose(f);
	return 1;
}

static void renderProgramCheckUpdate(RenderProgram *prog) {
	const char *fragment[RENDER_MAX_PROGRAM_SOURCES + 2];

	int updated = 0;
	int ready = 1;
	for (int i = 0; i < RENDER_MAX_PROGRAM_SOURCES; ++i) {
		if (prog->source_index[i] < 0) {
			fragment[i+1] = NULL;
			break;
		}

		RenderSource *src = g.scene->source + prog->source_index[i];
		updated |= sourceCheckUpdate(src);

		if (!src->processed_source) {
			MSG("Source %s is not ready", src->name);
			ready = 0;
			break;
		}

		fragment[i+1] = src->processed_source;
	}

	if (!ready) {
		MSG("Program %s is not ready", prog->name);
		return;
	}

	if (!updated)
		return;

	MutableString var_header;
	mutableStringInit(&var_header);
	mutableStringAppendZ(&var_header, "#version 130\n");
	for (int i = 0; i < RENDER_MAX_PROGRAM_SOURCES; ++i) {
		if (prog->source_index[i] < 0)
			break;
		const RenderSource *src = g.scene->source + prog->source_index[i];

		for (int j = 0; j < src->vars; ++j) {
			const VarDesc *v = src->var + j;
			const char *vtype = "";
			switch (v->type) {
			case VarType_None: /* FIXME assert */ continue;
			case VarType_Float: vtype = "float "; break;
			case VarType_Vec2: vtype = "vec2 "; break;
			case VarType_Vec3: vtype = "vec3 "; break;
			case VarType_Vec4: vtype = "vec4 "; break;
			}
			mutableStringAppendZ(&var_header, "uniform ");
			mutableStringAppendZ(&var_header, vtype);
			mutableStringAppendZ(&var_header, src->var[j].name);
			mutableStringAppendZ(&var_header, ";\n");
		}
	}

	if (prog->header)
		free((char*)prog->header);

	prog->header = mutableStringRelease(&var_header);
	fragment[0] = prog->header ? prog->header : "";

	MSG("%s", prog->header);

	MSG("Reloading %s", prog->name);

	const char *const vertex[] = { fs_vtx_source, NULL };
	renderReloadProgram(&prog->program, vertex, fragment);
}

void videoPaint() {
	if (g.scene_source->updated) {
		renderUpdateScene();
		return;
	}

	if (g.output.program_source->sequence != g.output.program_sequence) {
		const char * vertex[] = { fs_vtx_source, NULL };
		const char * fragment[] = { g.output.program_source->bytes, NULL };
		renderReloadProgram(&g.output.program, vertex, fragment);
		g.output.program_sequence = g.output.program_source->sequence;
	}

	if (!g.scene)
		return;

	int can_draw = (g.output.program > 0);
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

	GL(Viewport(0, 0, g.width, g.height));
	for (int i = 0; i < g.scene->passs; ++i) {
		RenderPass *pass = g.scene->pass + i;
		if (pass->blend) {
			glEnable(GL_BLEND);
			glBlendEquation(GL_FUNC_ADD);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		} else {
			glDisable(GL_BLEND);
		}
		const RenderProgram *prog = g.scene->program + pass->program_index;

		const ATimeUs time_start = aAppTime();

		static const GLuint bufs[] = {
			GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3};

		if (pass->fb > 0) {
			GL(BindFramebuffer(GL_FRAMEBUFFER, pass->fb));
			GL(DrawBuffers(pass->targets, bufs));
		} else {
			GL(BindFramebuffer(GL_FRAMEBUFFER, g.output.fb));
			GL(DrawBuffers(1, bufs));
		}

		GL(UseProgram(prog->program));
		GL(Uniform1iv(glGetUniformLocation(prog->program, "S"), g.scene->textures, samplers));

		for (int i = 0; i < RENDER_MAX_PROGRAM_SOURCES; ++i) {
			if (prog->source_index[i] < 0)
				break;
			const RenderSource *src = g.scene->source + prog->source_index[i];

			for (int j = 0; j < src->vars; ++j) {
				const VarDesc *var = src->var + j;
				AVec4f v;
				varGet(var, &v);

				const int loc = glGetUniformLocation(prog->program, var->name);
				//MSG("%s %d %f %f %f %f", var->name, loc, v.x, v.y, v.z, v.w);
				if (loc < 0)
					continue;

				switch (var->type) {
				case VarType_None: continue;
				case VarType_Float: GL(Uniform1f(loc, v.x)); break;
				case VarType_Vec2: GL(Uniform2f(loc, v.x, v.y)); break;
				case VarType_Vec3: GL(Uniform3f(loc, v.x, v.y, v.z)); break;
				case VarType_Vec4: GL(Uniform4f(loc, v.x, v.y, v.z, v.w)); break;
				}
			} // for src->vars
		} // for src in program sources

		GL(Rects(-1,-1,1,1));
		GL(Finish());

		++pass->stats.frames;
		pass->stats.time += (aAppTime() - time_start) * 1e-6f;

		if (pass->stats.frames % 60 == 0) {
			if (i == 0) MSG("-----");
			MSG("Pass %d avg time %.2fms", i, 1000.f * pass->stats.time / pass->stats.frames);
			pass->stats.time = 0;
			pass->stats.frames = 0;
		}
	}

	GL(BindFramebuffer(GL_FRAMEBUFFER, 0));
	GL(Viewport(0, 0, g.output.width, g.output.height));

	GL(UseProgram(g.output.program));
	GL(Uniform1iv(glGetUniformLocation(g.output.program, "S"), g.scene->textures, samplers));
	GL(Uniform1i(glGetUniformLocation(g.output.program, "OUTPUT"), RENDER_MAX_TEXTURES));

	int Vloc = glGetUniformLocation(g.output.program, "VIEWPORT");
	if (Vloc >= 0) {
		const float V[] = { (float)g.output.width, (float)g.output.height };
		GL(Uniform2fv(Vloc, 1, V));
	}

	GL(Rects(-1,-1,1,1));
}

void videoExport() {
	const RenderScene *scene = g.scene;
	if (!g.scene) {
		MSG("Render is not ready");
		return;
	}

	for (int i = 0; i < scene->sources; ++i) {
		renderSourceExport(scene->source + i);
	}
}
