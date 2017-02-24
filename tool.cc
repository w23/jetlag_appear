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
#include <vector>

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

class IResource {
public:
	virtual ~IResource() {}
	virtual bool update() { return false; }
};

class ResourceString : public IResource {
protected:
	std::string string_;
public:
	ResourceString(const std::string &str) : string_(str) {}
	const std::string &str() const { return string_; }
	bool update() override { return false; }
};

class ResourceStringFile : public ResourceString {
	FileChangePoller poller_;
public:
	ResourceStringFile(const char *filename) : ResourceString(""), poller_(filename) {}
	bool update() override {
		std::string new_content = poller_.poll();
		if (!new_content.empty()) {
			string_ = std::move(new_content);
			return true;
		}
		return false;
	}
};

class ResourceProgram : public IResource {
	ResourceString *vertex_, *fragment_;
	AGLProgram program_;
public:
	ResourceProgram(ResourceString *vertex, ResourceString *fragment)
		: vertex_(vertex), fragment_(fragment), program_(0) {}
	~ResourceProgram() {
		aGLProgramDestroy(program_);
	}
	bool update() override {
		if (vertex_->update() || fragment_->update()) {
			AGLProgram new_program = aGLProgramCreateSimple(
					vertex_->str().c_str(), fragment_->str().c_str());
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

	AGLProgram program() const { return program_; }
};

class ResourceValue : public IResource {
public:
protected:
	AGLAttributeType type_;
	union {
		float f;
		AVec2f v2;
	} v_;
	bool dirty_;
public:
	explicit ResourceValue(float f) : type_(AGLAT_Float), dirty_(false) { v_.f = f; }
	explicit ResourceValue(AVec2f f) : type_(AGLAT_Vec2), dirty_(false) { v_.v2 = f; }
	void update(float f) { v_.f = f; dirty_ = true; }
	void update(AVec2f f) { v_.v2 = f; dirty_ = true; }
	bool update() override { if (dirty_) { dirty_ = false; return true; } return false; }

	void fillUniform(AGLProgramUniform *uniform) {
		uniform->value.pf = &v_.f;
		uniform->count = 1;
		uniform->type = type_;
	}
};

static const float fsquad_vertices[] = {
	-1.f, 1.f,
	-1.f, -1.f,
	1.f, 1.f,
	1.f, -1.f
};

class ResourceDraw : public IResource {
protected:
	AGLDrawSource src_;
	AGLDrawMerge merge_;

	ResourceProgram *program_;

	std::vector<AGLAttribute> attribs_;
	std::vector<AGLProgramUniform> uniform_;

	struct UniformResource {
		std::string name;
		ResourceValue *value;
	};
	std::vector<UniformResource> uni_values_;

public:
	ResourceDraw(ResourceProgram *program) : program_(program) {
		merge_.blend.enable = 0;
		merge_.depth.mode = AGLDM_Disabled;

		src_.primitive.mode = GL_TRIANGLE_STRIP;
		src_.primitive.count = 4;
		src_.primitive.first = 0;
		src_.primitive.index_buffer = 0;
		src_.primitive.indices_ptr = 0;
		src_.primitive.index_type = 0;

		/* FIXME */
		AGLAttribute attr;
		attr.name = "av2_pos";
		attr.buffer = 0;
		attr.size = 2;
		attr.type = GL_FLOAT;
		attr.normalized = GL_FALSE;
		attr.stride = 0;
		attr.ptr = fsquad_vertices;
		addAttrib(attr);
	}

	void addAttrib(const AGLAttribute& attrib) {
		/* FIXME */
		attribs_.push_back(attrib);

		src_.attribs.p = &attribs_.front();
		src_.attribs.n = attribs_.size();
	}

	void addUniform(const char *name, ResourceValue *value) {
		UniformResource res;
		res.name = name;
		res.value = value;

		AGLProgramUniform uniform;
		uniform.name = res.name.c_str();
		value->fillUniform(&uniform);
		uniform_.push_back(std::move(uniform));
		uni_values_.push_back(std::move(res));

		src_.uniforms.p = &uniform_.front();
		src_.uniforms.n = uniform_.size();
	}

	bool update() override {
		bool updated = false;
		if (program_->update()) {
			src_.program = program_->program();
			updated = true;
		}

		for (auto it: uni_values_)
			updated |= it.value->update();

		return updated;
	}

	void draw(const AGLDrawTarget &target) {
		aGLDraw(&src_, &merge_, &target);
	};
};

class ResourceFramebuffer : public IResource {
	AGLDrawTarget target_;

	std::vector<ResourceDraw*> draws_;
	bool dirty_;

public:
	ResourceFramebuffer() : dirty_(false) {
		target_.framebuffer = 0; /* TODO */
	}
	~ResourceFramebuffer() { /* TODO */ }

	void resize(int x, int y, int w, int h) {
		target_.viewport.x = x;
		target_.viewport.y = y;
		target_.viewport.w = w;
		target_.viewport.h = h;

		dirty_ = true;
	}

	void addDraw(ResourceDraw *draw) {
		draws_.push_back(draw);
		dirty_ = true;
	}

	bool update() override {
		bool updated = dirty_;
		for (auto it: draws_)
			updated |= it->update();

		if (updated) {
			/* TODO parametrize */
			AGLClearParams clear;
			clear.a = 0;
			clear.r = 0;
			clear.g = 0;
			clear.b = 0;
			clear.depth = 0;
			clear.bits = AGLCB_Everything;
			aGLClear(&clear, &target_);

			for (auto it: draws_)
				it->draw(target_);
		}

		dirty_ = false;
		return updated;
	}
};

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

class Scene {
	ResourceString fs_vertex_;
	ResourceStringFile fragment_;
	ResourceProgram program_;
	ResourceValue time_;
	ResourceValue resolution_;
	ResourceDraw draw_;
	ResourceFramebuffer screen_;

public:
	Scene(const char *fragment_filename)
		: fs_vertex_(shader_vertex)
		, fragment_(fragment_filename)
		, program_(&fs_vertex_, &fragment_)
		, time_(0.f)
		, resolution_(aVec2f(16.f, 16.f))
		, draw_(&program_)
		, screen_()
	{
		draw_.addUniform("uf_time", &time_);
		draw_.addUniform("uv2_resolution", &resolution_);
		screen_.addDraw(&draw_);
	}

	void resize(unsigned w, unsigned h) {
		screen_.resize(0, 0, w, h);
		resolution_.update(aVec2f(w, h));
	}

	void draw(float t) {
		time_.update(t);
		screen_.update();
	}
};

static Scene *scene;

static void init(void) {
	scene = new Scene(a_app_state->argv[1]);
}

static void resize(ATimeUs timestamp, unsigned int old_w, unsigned int old_h) {
	(void)(timestamp); (void)(old_w); (void)(old_h);
	scene->resize(a_app_state->width, a_app_state->height);
}

static void paint(ATimeUs timestamp, float dt) {
	float t = timestamp * 1e-6f;
	(void)(dt);
	scene->draw(t);
}

void attoAppInit(struct AAppProctable *proctable) {
	aGLInit();
	init();

	proctable->resize = resize;
	proctable->paint = paint;
	proctable->key = keyPress;
}

