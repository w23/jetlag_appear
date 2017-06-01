#ifndef XRES
#define XRES 1280
#endif
#ifndef YRES
#define YRES 720
#endif
//#define DO_RANGES

#ifdef _DEBUG
#ifdef FULLSCREEN
#undef FULLSCREEN
#endif
#define DEBUG_FUNCLOAD
#define SHADER_DEBUG
#endif

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_LEAN
#define VC_LEANMEAN
#define VC_EXTRALEAN

#include <windows.h>
#include <mmsystem.h>
#include <mmreg.h>
#include <GL/gl.h>
#if defined(CAPTURE)
#include <stdio.h>
#ifdef __linux__
#include <unistd.h>
#endif
#endif

#ifdef _DEBUG
//#pragma data_seg(".fltused")
int _fltused = 1;
#endif

#define oglCreateShaderProgramv gl.CreateShaderProgramv
#define oglGenFramebuffers gl.GenFramebuffers
#define oglBindFramebuffer gl.BindFramebuffer
#define oglFramebufferTexture2D gl.FramebufferTexture2D
#define oglUseProgram gl.UseProgram
#define oglGetUniformLocation gl.GetUniformLocation
#define oglUniform1i gl.Uniform1i
#define oglUniform3f gl.Uniform3f

#elif defined(__linux__)
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>

#define __stdcall
#define __forceinline inline __attribute__((always_inline))

#define oglCreateShaderProgramv glCreateShaderProgramv
#define oglGenFramebuffers glGenFramebuffers
#define oglBindFramebuffer glBindFramebuffer
#define oglFramebufferTexture2D glFramebufferTexture2D
#define oglUseProgram glUseProgram
#define oglGetUniformLocation glGetUniformLocation
#define oglUniform1i glUniform1i
#define oglUniform3f glUniform3f
#endif

#include "glext.h"

//#include "music/4klang.h"
#define SAMPLE_RATE 44100
#define BPM 92.000000
#define MAX_INSTRUMENTS 8
#define MAX_PATTERNS 49
#define PATTERN_SIZE_SHIFT 4
#define PATTERN_SIZE (1 << PATTERN_SIZE_SHIFT)
#define MAX_TICKS (MAX_PATTERNS*PATTERN_SIZE)
#define SAMPLES_PER_TICK 7190
#define MAX_SAMPLES (SAMPLES_PER_TICK*MAX_TICKS)
#define POLYPHONY 2
#define FLOAT_32BIT
#define SAMPLE_TYPE float
#define INTRO_LENGTH (1000ull * MAX_SAMPLES / SAMPLE_RATE)

#ifdef CAPTURE
#ifndef CAPTURE_FRAMERATE
#define CAPTURE_FRAMERATE 60
#endif
#define LOL(x) #x
#define STR(x) LOL(x)
#define FFMPEG_CAPTURE_INPUT "ffmpeg.exe -y -f rawvideo -vcodec rawvideo -s "\
	STR(XRES) "x" STR(YRES) " -pix_fmt rgb24 -framerate " STR(CAPTURE_FRAMERATE)\
	" -i - -c:v libx264 -crf 18 -preset slow -vf vflip "\
	"capture_" STR(XRES) "x" STR(YRES) ".mp4"
#endif

#pragma data_seg(".raymarch.glsl")
#include "raymarch.h"

#pragma data_seg(".post.glsl")
#include "post.h"

#pragma data_seg(".timeline.h")
#include "timeline.h"

#pragma data_seg(".timeline_unpacked")
static float TV[TIMELINE_ROWS];

#pragma code_seg(".timeline_updater")
static __forceinline void timelineUpdate(float time) {
	int i, j;
	for (i = 1; i < TIMELINE_ROWS; ++i) {
		const float dt = timeline_times[i] * (float)(SAMPLES_PER_TICK) / (float)(SAMPLE_RATE) / 2.f;
		//printf("%.2f: [i=%d; dt=%.2f, t=%.2f]\n", time, i, dt, time/dt);
		if (dt >= time) {
			time /= dt;
			break;
		}
		time -= dt;
	}

	for (j = 0; j < TIMELINE_COLS; ++j) {
		const float a = timeline_values[i + j * TIMELINE_ROWS - 1];
		const float b = timeline_values[i + j * TIMELINE_ROWS];
#ifdef DO_RANGES
		const float r = timeline_ranges[j];
		TV[j] = ((a + (b - a) * time) / 255.f * 2.f - 1.f) * r;
#else
		TV[j] = ((a + (b - a) * time) - 127.f) / 2.f;
#endif
		//printf("%.2f ", TV[j]);
	}
	//printf("\n");
}

#ifdef NO_CREATESHADERPROGRAMV
FUNCLIST_DO(PFNGLCREATESHADERPROC, CreateShader) \
FUNCLIST_DO(PFNGLSHADERSOURCEPROC, ShaderSource) \
FUNCLIST_DO(PFNGLCOMPILESHADERPROC, CompileShader) \
FUNCLIST_DO(PFNGLCREATEPROGRAMPROC, CreateProgram) \
FUNCLIST_DO(PFNGLATTACHSHADERPROC, AttachShader) \
FUNCLIST_DO(PFNGLLINKPROGRAMPROC, LinkProgram)
#endif

#define FUNCLIST \
  FUNCLIST_DO(PFNGLCREATESHADERPROGRAMVPROC, CreateShaderProgramv) \
  FUNCLIST_DO(PFNGLUSEPROGRAMPROC, UseProgram) \
  FUNCLIST_DO(PFNGLGETUNIFORMLOCATIONPROC, GetUniformLocation) \
  FUNCLIST_DO(PFNGLUNIFORM1IPROC, Uniform1i) \
  FUNCLIST_DO(PFNGLUNIFORM3FPROC, Uniform3f) \
  FUNCLIST_DO(PFNGLGENFRAMEBUFFERSPROC, GenFramebuffers) \
  FUNCLIST_DO(PFNGLBINDFRAMEBUFFERPROC, BindFramebuffer) \
  FUNCLIST_DO(PFNGLFRAMEBUFFERTEXTURE2DPROC, FramebufferTexture2D)
#ifndef DEBUG
#define FUNCLIST_DBG
#else
#define FUNCLIST_DBG \
  FUNCLIST_DO(PFNGLGETPROGRAMINFOLOGPROC, GetProgramInfoLog) \
  FUNCLIST_DO(PFNGLGETSHADERINFOLOGPROC, GetShaderInfoLog) \
  FUNCLIST_DO(PFNGLCHECKFRAMEBUFFERSTATUSPROC, CheckFramebufferStatus)
#endif

#define FUNCLISTS FUNCLIST FUNCLIST_DBG

static SAMPLE_TYPE lpSoundBuffer[MAX_SAMPLES * 2];
static int p_ray, p_dof;
enum { FbTex_Ray, FbTex_COUNT };
static GLuint tex[FbTex_COUNT], fb[FbTex_COUNT];

#ifdef CAPTURE
static GLubyte backbufferData[XRES*YRES * 3];
#endif

#ifdef _WIN32
#pragma data_seg(".gl_names")
#define FUNCLIST_DO(T,N) "gl" #N "\0"
static const char gl_names[] =
FUNCLISTS
"\0";

#undef FUNCLIST_DO
#define FUNCLIST_DO(T,N) T N;
static struct {
	FUNCLISTS
} gl;
#undef FUNCLIST_DO

#pragma data_seg(".pfd")
static const PIXELFORMATDESCRIPTOR pfd = {
	sizeof(PIXELFORMATDESCRIPTOR), 1, PFD_DRAW_TO_WINDOW|PFD_SUPPORT_OPENGL|PFD_DOUBLEBUFFER, PFD_TYPE_RGBA,
	32, 0, 0, 0, 0, 0, 0, 8, 0, 0, 0, 0, 0, 0, 32, 0, 0, PFD_MAIN_PLANE, 0, 0, 0, 0 };

#pragma data_seg(".devmode")
static const DEVMODE screenSettings = { {0},
	#if _MSC_VER < 1400
	0,0,148,0,0x001c0000,{0},0,0,0,0,0,0,0,0,0,{0},0,32,XRES,YRES,0,0,      // Visual C++ 6.0
	#else
	0,0,156,0,0x001c0000,{0},0,0,0,0,0,{0},0,32,XRES,YRES,{0}, 0,           // Visual Studio 2005+
	#endif
	#if(WINVER >= 0x0400)
	0,0,0,0,0,0,
	#if (WINVER >= 0x0500) || (_WIN32_WINNT >= 0x0400)
	0,0
	#endif
	#endif
	};

#pragma data_seg(".wavefmt")
static const WAVEFORMATEX WaveFMT =
{
#ifdef FLOAT_32BIT
	WAVE_FORMAT_IEEE_FLOAT,
#else
	WAVE_FORMAT_PCM,
#endif
	2,                                   // channels
	SAMPLE_RATE,                         // samples per sec
	SAMPLE_RATE*sizeof(SAMPLE_TYPE) * 2, // bytes per sec
	sizeof(SAMPLE_TYPE) * 2,             // block alignment;
	sizeof(SAMPLE_TYPE) * 8,             // bits per sample
	0                                    // extension not needed
};

#pragma data_seg(".wavehdr")
static WAVEHDR WaveHDR =
{
	(LPSTR)lpSoundBuffer, MAX_SAMPLES*sizeof(SAMPLE_TYPE)*2,0,0,0,0,0,0
};

/*
static MMTIME MMTime =
{
	TIME_SAMPLES, 0
};
*/
#endif /* _WIN32 */

#pragma code_seg(".compileProgram")
static __forceinline int compileProgram(const char *fragment) {
#ifdef NO_CREATESHADERPROGRAMV
	const int pid = oglCreateProgram();
	const int fsId = oglCreateShader(GL_FRAGMENT_SHADER);
	oglShaderSource(fsId, 1, &fragment, 0);
	oglCompileShader(fsId);

#ifdef SHADER_DEBUG
	int result;
	char info[2048];
#define oglGetObjectParameteriv ((PFNGLGETOBJECTPARAMETERIVARBPROC) wglGetProcAddress("glGetObjectParameterivARB"))
#define oglGetInfoLog ((PFNGLGETINFOLOGARBPROC) wglGetProcAddress("glGetInfoLogARB"))
	oglGetObjectParameteriv(fsId, GL_OBJECT_COMPILE_STATUS_ARB, &result);
	oglGetInfoLog(fsId, 2047, NULL, (char*)info);
	if (!result)
	{
		MessageBox(NULL, info, "COMPILE", 0x00000000L);
		ExitProcess(0);
	}
#endif

	oglAttachShader(pid, fsId);
	oglLinkProgram(pid);

#else
	const int pid = oglCreateShaderProgramv(GL_FRAGMENT_SHADER, 1, &fragment);
#endif

#ifdef SHADER_DEBUG
	{
		int result;
		char info[2048];
#define oglGetObjectParameteriv ((PFNGLGETOBJECTPARAMETERIVARBPROC) wglGetProcAddress("glGetObjectParameterivARB"))
#define oglGetInfoLog ((PFNGLGETINFOLOGARBPROC) wglGetProcAddress("glGetInfoLogARB"))
		oglGetObjectParameteriv(pid, GL_OBJECT_LINK_STATUS_ARB, &result);
		oglGetInfoLog(pid, 2047, NULL, (char*)info);
		if (!result)
		{
			MessageBox(NULL, info, "LINK", 0x00000000L);
			ExitProcess(0);
		}
	}
#endif
	return pid;
}

static __forceinline void initFbTex(int fb, int tex) {
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, XRES, YRES, 0, GL_RGBA, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	oglBindFramebuffer(GL_FRAMEBUFFER, fb);
	oglFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);
}

static void paint(int prog, int src_tex, int dst_fb, float time) {
	oglUseProgram(prog);
	glBindTexture(GL_TEXTURE_2D, src_tex);
	oglBindFramebuffer(GL_FRAMEBUFFER, dst_fb);
	oglUniform1i(oglGetUniformLocation(prog, "B"), 0);
	oglUniform3f(oglGetUniformLocation(prog, "V"), (float)XRES, (float)YRES, time);
	oglUniform3f(oglGetUniformLocation(prog, "C"), TV[0], TV[1], TV[2]);
	oglUniform3f(oglGetUniformLocation(prog, "A"), TV[3], TV[4], TV[5]);
	oglUniform3f(oglGetUniformLocation(prog, "D"), TV[6], TV[7], TV[8]);
	//int i; for (i = 0; i < 9; ++i) printf("%f ", TV[i]); printf("\n");
#if defined(CAPTURE) && defined(TILED)
	{
		int x, y;
		for (y = 0; y < YRES; y += 64)
			for (x = 0; x < XRES; x += 64) {
				glRectf(
					x * 2.f / XRES - 1,
					y * 2.f / YRES - 1,
					(x + 64) * 2.f / XRES - 1,
					(y + 64) * 2.f / YRES - 1);
				glFinish();
			}
	}
#else
	glRects(-1, -1, 1, 1);
#endif
}

static __forceinline void introInit() {
	p_ray = compileProgram(raymarch_glsl);
	p_dof = compileProgram(post_glsl);
	//const int p_out = compileProgram(out_glsl);

	glGenTextures(FbTex_COUNT, tex);
	oglGenFramebuffers(FbTex_COUNT, fb);

	initFbTex(tex[FbTex_Ray], fb[FbTex_Ray]);
	//initFbTex(tex[FbTex_Dof], fb[FbTex_Dof]);
}

static __forceinline void introPaint(float time) {
	timelineUpdate(time);
	paint(p_ray, 0, fb[FbTex_Ray], time);
	paint(p_dof, tex[FbTex_Ray], 0, time);
}

#ifdef _WIN32
#pragma code_seg(".entry")
void entrypoint(void) {
#ifdef FULLSCREEN
	ChangeDisplaySettings(&screenSettings, CDS_FULLSCREEN);
	ShowCursor(0);
	HDC hDC = GetDC(CreateWindow((LPCSTR)0xC018, 0, WS_POPUP | WS_VISIBLE, 0, 0, XRES, YRES, 0, 0, 0, 0));
#else
	HDC hDC = GetDC(CreateWindow("static", 0, WS_POPUP | WS_VISIBLE, 0, 0, XRES, YRES, 0, 0, 0, 0));
#endif

	// initalize opengl
	SetPixelFormat(hDC, ChoosePixelFormat(hDC, &pfd), &pfd);
	wglMakeCurrent(hDC, wglCreateContext(hDC));

	{
		const char *next_gl_func = gl_names;
		void **funcptr = &gl;
		while (next_gl_func[0] != '\0') {
			*funcptr = wglGetProcAddress(next_gl_func);
#ifdef DEBUG_FUNCLOAD
			if (!*funcptr) { \
				MessageBox(NULL, next_gl_func, "wglGetProcAddress", 0x00000000L); \
			}
#endif
			++funcptr;
			while (*(next_gl_func++) != '\0');
		}
	}

	introInit();

#ifdef CAPTURE
	FILE* captureStream = _popen(FFMPEG_CAPTURE_INPUT, "wb");
#else
#if 0
	// initialize sound
	HWAVEOUT hWaveOut;
	CreateThread(0, 0, (LPTHREAD_START_ROUTINE)_4klang_render, lpSoundBuffer, 0, 0);
	//_4klang_render(lpSoundBuffer);
	waveOutOpen(&hWaveOut, WAVE_MAPPER, &WaveFMT, NULL, 0, CALLBACK_NULL);
	waveOutPrepareHeader(hWaveOut, &WaveHDR, sizeof(WaveHDR));
	waveOutWrite(hWaveOut, &WaveHDR, sizeof(WaveHDR));
#endif
#endif

	const int to = timeGetTime();

	// play intro
	do {
		//waveOutGetPosition(hWaveOut, &MMTime, sizeof(MMTIME));
#ifdef CAPTURE
		static int frame = -1;
		frame++;
		const long time = (float)frame / CAPTURE_FRAMERATE * 1000.;
#else
		const int time = timeGetTime() - to;
#endif
		introPaint(time / 1000.f);
		SwapBuffers(hDC);

#ifdef CAPTURE
		glReadPixels(0, 0, XRES, YRES, GL_RGB, GL_UNSIGNED_BYTE, backbufferData);
		fwrite(backbufferData, 1, XRES*YRES * 3, captureStream);
		fflush(captureStream);
#endif

		/* hide cursor properly */
		PeekMessageA(0, 0, 0, 0, PM_REMOVE);
		if (time > INTRO_LENGTH) break;
	} while (!GetAsyncKeyState(VK_ESCAPE));
		// && MMTime.u.sample < MAX_SAMPLES);

	#ifdef CLEANDESTROY
	sndPlaySound(0,0);
	ChangeDisplaySettings( 0, 0 );
	ShowCursor(1);
	#endif

#ifdef CAPTURE
	fclose(captureStream);
#endif

	ExitProcess(0);
}
#elif defined(__linux__)
/* cc -Wall -Wno-unknown-pragmas `pkg-config --cflags --libs sdl` -lGL nwep.c -o nwep */
#include <SDL.h>
#include <pthread.h>

#ifndef NO_AUDIO
void __4klang_render(void*);

static int audio_cursor = 0;

static void audioPlay(void *userdata, Uint8 *stream, int len) {
	short *p = (short*)stream;
	int i;
	len /= sizeof(short);
	for (i = 0; i < len; ++i) {
		audio_cursor = (audio_cursor + 1) % (MAX_SAMPLES * 2);
		p[i] = lpSoundBuffer[audio_cursor] * 32767.f;
	}
}

static SDL_AudioSpec as = {SAMPLE_RATE, 0x8010, 2, 0, 0, 0, 0, audioPlay};
#endif /* AUDIO */

#ifndef COMPACT
int main() {
#else
void _start() {
#endif
#ifndef NO_AUDIO
	pthread_t audio_thread;
	pthread_create(&audio_thread, 0, (void *(*)(void*))__4klang_render, lpSoundBuffer);
	//__4klang_render(lpSoundBuffer);
#endif
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
#ifdef FULLSCREEN
#undef FULLSCREEN
#define FULLSCREEN SDL_FULLSCREEN
	SDL_ShowCursor(0);
#endif
	SDL_SetVideoMode(XRES, YRES, 32, SDL_OPENGL | FULLSCREEN);
	glViewport(0, 0, XRES, YRES);
	introInit();

#ifndef CAPTURE
#ifndef NO_AUDIO
	SDL_OpenAudio(&as, 0);
	SDL_PauseAudio(0);
#endif
	const uint32_t start = SDL_GetTicks();
	for(;;) {
		const uint32_t now = SDL_GetTicks();
#else
	const uint32_t global_start = SDL_GetTicks();
	const uint32_t start = 0;
	const uint32_t total_frames = INTRO_LENGTH * CAPTURE_FRAMERATE / 1000;
	uint32_t frame_start = SDL_GetTicks();
	int frame = 0;
	for (;;) {
		const uint32_t now = ((frame++) * 1000.f) / CAPTURE_FRAMERATE;
#endif
		SDL_Event e;
		SDL_PollEvent(&e);
		if ((e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) || now >= INTRO_LENGTH) break;
		introPaint((now - start) / 1000.f);

#ifdef CAPTURE
		const uint32_t frame_end = SDL_GetTicks();
		const uint32_t total_time = frame_end - global_start;
		glReadPixels(0, 0, XRES, YRES, GL_RGB, GL_UNSIGNED_BYTE, backbufferData);
		write(1, backbufferData, XRES * YRES * 3);
		fprintf(stderr, "frame %d/%d: %dms/frame, total: %ds/%llds, MBytes: %lld/%lld\n",
			frame, total_frames, frame_end - frame_start,
			total_time / 1000, (unsigned long long)total_time * total_frames / frame / 1000,
			((unsigned long long)frame * XRES * YRES * 3) / 1024 / 1024,
			((unsigned long long)total_frames * XRES * YRES * 3) / 1024 / 1024);
		usleep(1000);
		frame_start = frame_end;
#endif

		SDL_GL_SwapBuffers();
	}

#ifdef COMPACT
	asm ( \
		"xor %eax,%eax\n" \
		"inc %eax\n" \
		"int $0x80\n" \
	);
#else
	SDL_Quit();
	return 0;
#endif
}
#else
#error Not ported
#endif
