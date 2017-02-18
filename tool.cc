#include "atto/app.h"
#define ATTO_GL_H_IMPLEMENT
#include "atto/gl.h"
#include "atto/math.h"

#include <math.h>

#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <string>
#include <memory>

static void keyPress(ATimeUs timestamp, AKey key, int pressed) {
	(void)(timestamp); (void)(pressed);
	if (key == AK_Esc)
		aAppTerminate(0);
}

static const char shader_vertex[] =
	"attribute vec2 av2_pos;\n"
	"varying vec2 vv2_pos;\n"
	"void main() {\n"
		"vv2_pos = av2_pos;\n"
		"gl_Position = vec4(av2_pos, 0., 1.);\n"
	"}"
;

static const float vertices[] = {
	-1.f, 1.f,
	-1.f, -1.f,
	1.f, 1.f,
	1.f, -1.f
};

enum Uniform {
	UniformTime,
	UniformResolution,
	Uniform_count
};

class FileChangePoller {
	const std::string filename_;
	timespec mtime_;

public:
	FileChangePoller(const char *filename) : filename_(filename) {
		mtime_.tv_sec = 0;
		mtime_.tv_nsec = 0;
	}
	~FileChangePoller() {}

	std::string poll() {
		std::string content;
		struct stat st;
		stat(filename_.c_str(), &st);
		if (st.st_mtim.tv_sec == mtime_.tv_sec &&
				st.st_mtim.tv_nsec == mtime_.tv_nsec)
			return content;

		aAppDebugPrintf("Updating..");

		mtime_ = st.st_mtim;

		int fd = open(filename_.c_str(), O_RDONLY);
		if (fd < 0) {
			aAppDebugPrintf("Cannot open file");
			return content;
		}

		content.resize(st.st_size + 1, 0);

		if (read(fd, &content[0], st.st_size) != st.st_size) {
			aAppDebugPrintf("Cannot read file\n");
			content.resize(0);
			return content;
		}

		close(fd);
		return content;
	}
};

class ProgramPoller {
	FileChangePoller file_;
	AGLProgram program_;

public:
	ProgramPoller(const char *filename) : file_(filename), program_(0) {}
	~ProgramPoller() {}

	void poll() {
		std::string sources = file_.poll();
		if (!sources.empty()) {
			AGLProgram new_program = aGLProgramCreateSimple(shader_vertex, sources.c_str());
			if (new_program < 0) {
				aAppDebugPrintf("shader error: %s\n", a_gl_error);
				return;
			}

			if (program_ > 0)
				aGLProgramDestroy(program_);

			program_ = new_program;
		}
	}

	AGLProgram program() const { return program_; }
};

class Pass {
	AGLDrawSource src_;
	AGLDrawMerge merge_;
	AGLDrawTarget target_;

	AGLTexture targe_texture_;

public:
	Pass(const char *filename);
	~Pass();
};

static struct {
	AGLAttribute attr[1];
	AGLProgramUniform pun[Uniform_count];

	AGLDrawSource draw_fsquad;
	AGLDrawMerge merge;
	AGLDrawTarget target_screen;

	std::unique_ptr<ProgramPoller> fsprog;
	struct AVec2f resolution;
} g;

static void init(void) {
	g.draw_fsquad.program = 0;

	g.attr[0].name = "av2_pos";
	g.attr[0].buffer = 0;
	g.attr[0].size = 2;
	g.attr[0].type = GL_FLOAT;
	g.attr[0].normalized = GL_FALSE;
	g.attr[0].stride = 0;
	g.attr[0].ptr = vertices;

	g.pun[UniformTime].name = "uf_time";
	g.pun[UniformTime].type = AGLAT_Float;
	g.pun[UniformTime].count = 1;

	g.pun[UniformResolution].name = "uv2_resolution";
	g.pun[UniformResolution].type = AGLAT_Vec2;
	g.pun[UniformResolution].count = 1;
	g.pun[UniformResolution].value.pf = &g.resolution.x;

	g.draw_fsquad.primitive.mode = GL_TRIANGLE_STRIP;
	g.draw_fsquad.primitive.count = 4;
	g.draw_fsquad.primitive.first = 0;
	g.draw_fsquad.primitive.index_buffer = 0;
	g.draw_fsquad.primitive.indices_ptr = 0;
	g.draw_fsquad.primitive.index_type = 0;

	g.draw_fsquad.attribs.p = g.attr;
	g.draw_fsquad.attribs.n = sizeof g.attr / sizeof *g.attr;

	g.draw_fsquad.uniforms.p = g.pun;
	g.draw_fsquad.uniforms.n = sizeof g.pun / sizeof *g.pun;

	g.merge.blend.enable = 0;
	g.merge.depth.mode = AGLDM_Disabled;

	g.fsprog.reset(new ProgramPoller(a_app_state->argv[1]));
}

static void resize(ATimeUs timestamp, unsigned int old_w, unsigned int old_h) {
	(void)(timestamp); (void)(old_w); (void)(old_h);
	g.target_screen.viewport.x = g.target_screen.viewport.y = 0;
	g.target_screen.viewport.w = a_app_state->width;
	g.target_screen.viewport.h = a_app_state->height;

	g.target_screen.framebuffer = 0;

	g.resolution = aVec2f(a_app_state->width, a_app_state->height);
}

static void paint(ATimeUs timestamp, float dt) {
	float t = timestamp * 1e-6f;
	AGLClearParams clear;
	(void)(dt);

	clear.a = 1;
	clear.r = sinf(t*.1f);
	clear.g = sinf(t*.2f);
	clear.b = sinf(t*.3f);
	clear.depth = 0;
	clear.bits = AGLCB_Everything;

	aGLClear(&clear, &g.target_screen);

	if (g.fsprog) {
		g.fsprog->poll();
		g.draw_fsquad.program = g.fsprog->program();
		if (g.draw_fsquad.program) {
			g.pun[0].value.pf = &t;
			aGLDraw(&g.draw_fsquad, &g.merge, &g.target_screen);
		}
	}
}

void attoAppInit(struct AAppProctable *proctable) {
	aGLInit();
	init();

	proctable->resize = resize;
	proctable->paint = paint;
	proctable->key = keyPress;
}

