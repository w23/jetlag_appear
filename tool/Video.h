#pragma once

#include "Timeline.h"
#include "FileString.h"
#include "atto/gl.h"

class Program {
	String &vertex_src_;
	String &fragment_src_;
	AGLProgram program_;

public:
	const AGLProgram& program() const { return program_; }
	Program(String &vtx, String &frg) : vertex_src_(vtx), fragment_src_(frg), program_(0) {}

	bool update();
};

class Video {
public:
	Video(int width, int height);//const char *raymarch_file, const char *post_file)

	bool paint(const Timeline &timeline, float now, bool force_redraw);

private:
	void drawPass(float now, const Timeline::Sample &TV, int tex, int prog, int fb);

	int width, height;
	enum {
		FbTex_None,
		FbTex_Random,
		FbTex_Ray,
		FbTex_Frame,
		FbTex_COUNT
	};
	GLuint tex_[FbTex_COUNT];
	GLuint fb_[FbTex_COUNT];

	String fs_vtx;
	FileString raymarch_src;
	Program raymarch_prg;
	FileString post_src;
	Program post_prg;
	FileString out_src;
	Program out_prg;
}; // class Video
