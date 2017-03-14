#include "atto/app.h"
#define ATTO_GL_H_IMPLEMENT
#define ATTO_GL_DEBUG
//#define ATTO_GL_TRACE
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

class INode {
public:
	virtual ~INode() {}
	virtual bool update() { return false; }
};

class NodeString : public INode {
protected:
	std::string string_;
public:
	NodeString(const std::string &str) : string_(str) {}
	const std::string &str() const { return string_; }
};

class NodeStringFile : public NodeString {
	FileChangePoller poller_;
public:
	NodeStringFile(const char *filename) : NodeString(""), poller_(filename) {}
	bool update() override {
		std::string new_content = poller_.poll();
		if (!new_content.empty()) {
			string_ = std::move(new_content);
			return true;
		}
		return false;
	}
};

class NodeProgram : public INode {
	NodeString *vertex_, *fragment_;
	AGLProgram program_;
public:
	NodeProgram(NodeString *vertex, NodeString *fragment)
		: vertex_(vertex), fragment_(fragment), program_(0) {}
	~NodeProgram() {
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

class INodeUniform : public INode {
public:
	virtual void fillUniform(AGLProgramUniform *uniform) = 0;
};

class NodeUniformFloat : public INodeUniform {
	float f_;
	bool dirty_;
public:
	explicit NodeUniformFloat(float f) : f_(f), dirty_(false) {}
	void update(float f) { f_ = f; dirty_ = true; }
	bool update() override { if (dirty_) { dirty_ = false; return true; } return false; }
	void fillUniform(AGLProgramUniform *uniform) override {
		uniform->value.pf = &f_;
		uniform->count = 1;
		uniform->type = AGLAT_Float;
	}
};

class NodeUniformVec2 : public INodeUniform {
	AVec2f v_;
	bool dirty_;
public:
	explicit NodeUniformVec2(AVec2f v) : v_(v), dirty_(false) {}
	void update(AVec2f v) { v_ = v; dirty_ = true; }
	bool update() override { if (dirty_) { dirty_ = false; return true; } return false; }
	void fillUniform(AGLProgramUniform *uniform) override {
		uniform->value.pf = &v_.x;
		uniform->count = 1;
		uniform->type = AGLAT_Vec2;
	}
};

class NodeTexture : public INodeUniform {
protected:
	AGLTexture tex_;
	NodeUniformVec2 resolution_;
	bool dirty_;

public:
	explicit NodeTexture()
		: tex_(aGLTextureCreate()), resolution_(aVec2f(0,0)), dirty_(false) {}
	~NodeTexture() { aGLTextureDestroy(&tex_); }
	NodeUniformVec2 *resolution() { return &resolution_; } /* FIXME should depend on this texture */
	void upload(int w, int h, AGLTextureFormat fmt, void *pixels) {
		AGLTextureUploadData data;
		data.pixels = pixels;
		data.x = data.y = 0;
		data.width = w;
		data.height = h;
		data.format = fmt;
		aGLTextureUpload(&tex_, &data);
		resolution_.update(aVec2f(w, h));
		dirty_ = true;
	}
	const AGLTexture& agl() const { return tex_; }
	void fillUniform(AGLProgramUniform *uniform) override {
		uniform->value.texture = &tex_;
		uniform->count = 1;
		uniform->type = AGLAT_Texture;
	}
	bool update() override { if (dirty_) { dirty_ = false; return true; } return false; }
};

class Rand {
	uint64_t v_;
public:
	Rand(uint64_t seed) : v_(seed) {}
	uint32_t next() {
		v_ = v_ * 6364136223846793005ull + 1442695040888963407ull;
		return v_ >> 32;
	}
};

class NodeRandomTexture : public NodeTexture {
	
public:
	explicit NodeRandomTexture(int w, int h, int seed = 31337)
	{
		const int num_pixels = w * h;
		Rand rand(seed);

		std::vector<uint32_t> buffer(num_pixels);
		for (int i = 0; i < num_pixels; ++i)
			buffer[i] = rand.next();

		upload(w, h, AGLTF_U8_RGBA, &buffer[0]);
	}

	bool update() override { return false; }
};

static const float fsquad_vertices[] = {
	-1.f, 1.f,
	-1.f, -1.f,
	1.f, 1.f,
	1.f, -1.f
};

class NodeDraw : public INode {
protected:
	AGLDrawSource src_;
	AGLDrawMerge merge_;

	NodeProgram *program_;

	std::vector<AGLAttribute> attribs_;
	std::vector<AGLProgramUniform> uniform_;

	struct UniformNode {
		std::string name;
		INodeUniform *value;
	};
	std::vector<UniformNode> uni_values_;

protected:
	explicit NodeDraw() : program_(nullptr) {
		merge_.blend.enable = 0;
		merge_.depth.mode = AGLDM_Disabled;
	}

public:
	explicit NodeDraw(NodeProgram *program) : program_(program) {
		merge_.blend.enable = 0;
		merge_.depth.mode = AGLDM_Disabled;
	}

	void addAttrib(const AGLAttribute& attrib) {
		/* FIXME */
		attribs_.push_back(attrib);

		src_.attribs.p = &attribs_.front();
		src_.attribs.n = attribs_.size();
	}

	void addUniform(const char *name, INodeUniform *value) {
		UniformNode res;
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

class NodeDrawFullscreen : public NodeDraw {
	NodeString vertex_src_;
	NodeProgram own_program_;

	static const char shader_vertex_[];
	static const float fsquad_vertices_[];

public:
	NodeDrawFullscreen(NodeString *fragment_src)
		: NodeDraw()
		, vertex_src_(shader_vertex_)
		, own_program_(&vertex_src_, fragment_src) {
		program_ = &own_program_;

		src_.primitive.front_face = AGLFF_CounterClockwise;
		src_.primitive.cull_mode = AGLCM_Disable;
		src_.primitive.mode = GL_TRIANGLE_STRIP;
		src_.primitive.count = 4;
		src_.primitive.first = 0;
		src_.primitive.index.buffer = 0;
		src_.primitive.index.data.ptr = 0;
		src_.primitive.index.type = 0;

		AGLAttribute attr;
		attr.name = "av2_pos";
		attr.buffer = 0;
		attr.size = 2;
		attr.type = GL_FLOAT;
		attr.normalized = GL_FALSE;
		attr.stride = 0;
		attr.ptr = fsquad_vertices_;
		addAttrib(attr);
	}

	void addUniform(const char *name, INodeUniform *value) {
		NodeDraw::addUniform(name, value);
	}
};

const char NodeDrawFullscreen::shader_vertex_[] =
	"attribute vec2 av2_pos;\n"
	"varying vec2 vv2_pos;\n"
	"void main() {\n"
		"vv2_pos = av2_pos;\n"
		"gl_Position = vec4(av2_pos, 0., 1.);\n"
	"}"
;

const float NodeDrawFullscreen::fsquad_vertices_[] = {
	-1.f, 1.f,
	-1.f, -1.f,
	1.f, 1.f,
	1.f, -1.f
};

class NodeBaseFramebuffer : public INode {
	std::vector<NodeDraw*> draws_;

protected:
	AGLDrawTarget target_;
	bool dirty_;

public:
	NodeBaseFramebuffer() : dirty_(false) {
		target_.framebuffer = 0;
	}

	void resize(int w, int h) {
		target_.viewport.x = 0;
		target_.viewport.y = 0;
		target_.viewport.w = w;
		target_.viewport.h = h;

		dirty_ = true;
	}

	void addDraw(NodeDraw *draw) {
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

class NodeScreen : public NodeBaseFramebuffer {};

class NodeFramebuffer : public NodeBaseFramebuffer {
	class NodeFramebufferTexture : public NodeTexture {
		NodeFramebuffer& framebuffer_;
	public:
		NodeFramebufferTexture(NodeFramebuffer& fb) : framebuffer_(fb) {}
		bool update() override { return framebuffer_.update(); }
	};

	AGLFramebufferParams fb_;
	std::vector<std::pair<AGLTextureFormat, NodeFramebufferTexture*> > targets_;

public:
	NodeFramebuffer() {
		target_.viewport.w = target_.viewport.h = 16;
		target_.framebuffer = &fb_;
		dirty_ = true;
	}

	std::unique_ptr<NodeTexture> addTarget(AGLTextureFormat fmt) {
		ATTO_ASSERT(targets_.empty());
		NodeFramebufferTexture *tex = new NodeFramebufferTexture(*this);
		targets_.push_back(std::make_pair(fmt, tex));
		dirty_ = true;
		return std::unique_ptr<NodeTexture>(tex);
	}

	bool update() override {
		if (targets_.empty()) return false;

		if (dirty_) {
			for (auto tex: targets_) {
				tex.second->upload(target_.viewport.w, target_.viewport.h, tex.first, 0);
				fb_.color = &tex.second->agl();
			}

			fb_.depth.mode = AGLDBM_Texture;
			fb_.depth.texture = 0;
		}

		return NodeBaseFramebuffer::update();
	}
};

static void keyPress(ATimeUs timestamp, AKey key, int pressed) {
	(void)(timestamp); (void)(pressed);
	if (key == AK_Esc)
		aAppTerminate(0);
}

class Scene {
	NodeRandomTexture rand_tex_;

	NodeUniformFloat time_;
	NodeUniformVec2 resolution_;

	NodeStringFile ray_frag_;
	NodeDrawFullscreen ray_draw_;

	NodeFramebuffer framebuffer_;

	std::unique_ptr<NodeTexture> frame_tex_;
	NodeStringFile post_frag_;
	NodeDrawFullscreen post_draw_;

	NodeScreen screen_;

public:
	Scene(const char *ray_frag, const char *post_frag)
		: rand_tex_(256, 256, 1)
		, time_(0.f)
		, resolution_(aVec2f(1280.f, 720.f))
		, ray_frag_(ray_frag)
		, ray_draw_(&ray_frag_)
		, framebuffer_()
		, frame_tex_(framebuffer_.addTarget(AGLTF_F32_RGBA))
		, post_frag_(post_frag)
		, post_draw_(&post_frag_)
		, screen_()
	{
		framebuffer_.resize(1280, 720);

		ray_draw_.addUniform("uf_time", &time_);
		ray_draw_.addUniform("uv2_resolution", frame_tex_->resolution());
		ray_draw_.addUniform("uv2_rand_resolution", rand_tex_.resolution());
		ray_draw_.addUniform("us2_rand", &rand_tex_);

		framebuffer_.addDraw(&ray_draw_);

		post_draw_.addUniform("uv2_resolution", &resolution_);
		post_draw_.addUniform("us2_frame", frame_tex_.get());
		post_draw_.addUniform("uv2_frame_resolution", frame_tex_->resolution());
		post_draw_.addUniform("uv2_rand_resolution", rand_tex_.resolution());
		post_draw_.addUniform("us2_rand", &rand_tex_);
		screen_.addDraw(&post_draw_);
	}

	void resize(unsigned w, unsigned h) {
		screen_.resize(w, h);
		resolution_.update(aVec2f(w, h));
	}

	void draw(float t) {
		time_.update(t);
		screen_.update();
	}
};

static Scene *scene;

static void init(void) {
	scene = new Scene(a_app_state->argv[1], a_app_state->argv[2]);
}

static void resize(ATimeUs timestamp, unsigned int old_w, unsigned int old_h) {
	(void)(timestamp); (void)(old_w); (void)(old_h);
	scene->resize(a_app_state->width, a_app_state->height);
}

static void paint(ATimeUs timestamp, float dt) {
	float t = timestamp * 1e-6f;
	(void)(dt);
	scene->draw(t);
//	exit(0);
}

void attoAppInit(struct AAppProctable *proctable) {
	aGLInit();
	init();

	proctable->resize = resize;
	proctable->paint = paint;
	proctable->key = keyPress;
}

