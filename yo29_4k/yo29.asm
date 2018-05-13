BITS 32
global _entrypoint

%define AUDIO_THREAD
%define WIDTH 1280
%define HEIGHT 720
%define NOISE_SIZE 256
%define NOISE_SIZE_BYTES (4 * NOISE_SIZE * NOISE_SIZE)

;%include "4klang.inc"
%define USE_SECTIONS
%define SAMPLE_RATE	44100
%define MAX_INSTRUMENTS	10
%define MAX_VOICES 2
%define HLD 1
%define BPM 60.000000
%define MAX_PATTERNS 121
%define PATTERN_SIZE_SHIFT 3
%define PATTERN_SIZE (1 << PATTERN_SIZE_SHIFT)
%define	MAX_TICKS (MAX_PATTERNS*PATTERN_SIZE)
%define	SAMPLES_PER_TICK 11025
%define DEF_LFO_NORMALIZE 0.0000226757
%define	MAX_SAMPLES	(SAMPLES_PER_TICK*MAX_TICKS)
extern __4klang_render@4

%ifndef DEBUG
%define FULLSCREEN
%define GLCHECK
%else
%macro GLCHECK 0
	call glGetError
	test eax, eax
	jz %%ok
	int 3
%%ok:
%endmacro
%endif

GL_TEXTURE_2D EQU 0x0de1
GL_FRAGMENT_SHADER EQU 0x8b30
GL_UNSIGNED_BYTE EQU 0x1401
GL_FLOAT EQU 0x1406
GL_RGBA EQU 0x1908
GL_LINEAR EQU 0x2601
GL_TEXTURE_MIN_FILTER EQU 0x2801
GL_RGBA16F EQU 0x881a
GL_BLEND EQU 0x0be2
GL_SRC_ALPHA EQU 0x0302
GL_ONE_MINUS_SRC_ALPHA EQU 0x0303

%macro WINAPI_FUNCLIST 0
%ifdef FULLSCREEN
	WINAPI_FUNC ChangeDisplaySettingsA, 8
%endif
%ifdef AUDIO_THREAD
	WINAPI_FUNC CreateThread, 24
%endif
	WINAPI_FUNC waveOutOpen, 24
	WINAPI_FUNC waveOutWrite, 12
	WINAPI_FUNC waveOutGetPosition, 12
	WINAPI_FUNC ShowCursor, 4
	WINAPI_FUNC CreateWindowExA, 48
	WINAPI_FUNC GetDC, 4
	WINAPI_FUNC ChoosePixelFormat, 8
	WINAPI_FUNC SetPixelFormat, 12
	WINAPI_FUNC wglCreateContext, 4
	WINAPI_FUNC wglMakeCurrent, 8
	WINAPI_FUNC wglGetProcAddress, 4
	WINAPI_FUNC SwapBuffers, 4
	WINAPI_FUNC PeekMessageA, 20
	WINAPI_FUNC GetAsyncKeyState, 4
	WINAPI_FUNC glGenTextures, 8
	WINAPI_FUNC glBindTexture, 8
	WINAPI_FUNC glTexImage2D, 36
	WINAPI_FUNC glTexParameteri, 12
	WINAPI_FUNC glRects, 16
	WINAPI_FUNC glEnable, 4
	WINAPI_FUNC glBlendFunc, 8
	WINAPI_FUNC glGetError, 0
%if 0
	WINAPI_FUNC glClearColor, 16
	WINAPI_FUNC glClear, 4
%endif
	WINAPI_FUNC ExitProcess, 4
%endmacro

%macro WINAPI_FUNC 2
	extern _ %+ %1 %+ @ %+ %2
	%1 EQU _ %+ %1 %+ @ %+ %2
%endmacro
WINAPI_FUNCLIST
%unmacro WINAPI_FUNC 2

%macro GL_FUNCLIST 0
	GL_FUNC glCreateShaderProgramv
	GL_FUNC glUseProgram
	GL_FUNC glGetUniformLocation
	GL_FUNC glUniform1i
%endmacro

section .dglstr data align=1
gl_proc_names:
%macro GL_FUNC 1
%defstr %[%1 %+ __str] %1
	db %1 %+ __str, 0
%endmacro
GL_FUNCLIST
%unmacro GL_FUNC 1
	db 0

;vm_procs:
;%macro WINAPI_FUNC 2
;  %1 EQU _ %+ %1 %+ @ %+ %2
;%endmacro
;WINAPI_FUNCLIST
;%unmacro WINAPI_FUNC 2

section .bglproc bss
gl_procs:
%macro GL_FUNC 1
%1 %+ _:
%define %1 [%1 %+ _]
	resd 1
%endmacro
GL_FUNCLIST
%unmacro GL_FUNC 1

section .dsmain data
%include "main.shader.inc"

section .dpfd data
pfd:
	DW	028H, 01H
	DD	025H
	DB	00H, 020H, 00H, 00H, 00H, 00H, 00H, 00H, 08H, 00H, 00H, 00H, 00H, 00H
	DB	00H, 020H, 00H, 00H, 00H, 00H
	DD	00H, 00H, 00H

%ifdef FULLSCREEN
section .ddevmod data
devmode:
	times 9 dd 0
	db 0x9c, 0, 0, 0
	db 0, 0, 0x1c, 0
	times 15 dd 0
	DD	020H, WIDTH, HEIGHT
	times 10 dd 0
%endif

section .dstrs data
%ifdef DEBUG
static: db "static", 0
%endif
S: db 'S', 0
F: db 'F', 0

section .dwvfmt data
wavefmt:
	dw 3 ; wFormatTag = WAVE_FORMAT_IEEE_FLOAT
	dw 2 ; nChannels
	dd SAMPLE_RATE ; nSamplesPerSec
	dd SAMPLE_RATE * 4 * 2; nAvgBytesPerSec
  dw 4 * 2 ; nBlockAlign
  dw 8 * 4 ; wBitsPerSample
  dw 0 ; cbSize

section .dwvhdr data
wavehdr:
	dd sound_buffer ; lpData
	dd MAX_SAMPLES * 2 * 4 ; dwBufferLength
	times 2 dd 0 ; unused stuff
	;dd 0 ; dwFlags TODO WHDR_PREPARED   0x00000002
	dd 2 ; dwFlags WHDR_PREPARED  =  0x00000002
	times 4 dd 0 ; unused stuff
	wavehdr_size EQU ($ - wavehdr)

section .bsndbuf bss
sound_buffer: resd MAX_SAMPLES * 2

section .bnoise bss
dev_null: resd 1
noise: resb NOISE_SIZE_BYTES

section .dsptrs data
src_main:
	dd _main_shader_glsl

section .dsmplrs data
samplers:
	dd 0

tex_noise EQU 1
prog_main EQU 1

%macro initTexture 6
	push %1
	push GL_TEXTURE_2D
	call glBindTexture

	push %6
	push %5
	push GL_RGBA
	push 0
	push %3
	push %2
	push %4
	push 0
	push GL_TEXTURE_2D
	call glTexImage2D

	push GL_LINEAR
	push GL_TEXTURE_MIN_FILTER
	push GL_TEXTURE_2D
	call glTexParameteri
%endmacro

%macro initTextureStack 6
	push GL_LINEAR
	push GL_TEXTURE_MIN_FILTER
	push GL_TEXTURE_2D
	;call glTexParameteri

	push %6
	push %5
	push GL_RGBA
	push 0
	push %3
	push %2
	push %4
	push 0
	push GL_TEXTURE_2D
	;call glTexImage2D

	push %1
	push GL_TEXTURE_2D
	;call glBindTexture
%endmacro


section .centry text align=1
_entrypoint:
	xor ecx, ecx

	; glGenTextures
	push dev_null
	push 1
	; SetPixelFormat
	push pfd
	; ChoosePixelFormat
	push pfd
	push ecx
	push ecx
	push ecx
	push ecx
	push HEIGHT
	push WIDTH
	push ecx
	push ecx
	push 0x90000000
	push ecx
%ifdef DEBUG
	push static
%else
	push 0xc018
%endif
	push ecx
	push ecx

%if 1
	;CHECK(waveOutOpen(&hWaveOut, WAVE_MAPPER, &WaveFMT, NULL, 0, CALLBACK_NULL));
	push ecx
	push ecx
	push ecx
	push wavefmt
	push -1
	push noise

	;CHECK(waveOutPrepareHeader(hWaveOut, &WaveHDR, sizeof(WaveHDR)));
	;push wavehdr_size
	;push wavehdr
%endif

%ifdef FULLSCREEN
	push 4
	push devmode
%endif

;	CreateThread(0, 0, (LPTHREAD_START_ROUTINE)soundRender, sound_buffer, 0, 0);
%ifdef AUDIO_THREAD
	push ecx
	push ecx
	push sound_buffer
	push __4klang_render@4
	push ecx
	push ecx
	call CreateThread
%else
	push sound_buffer
	call __4klang_render@4
%endif

generate_noise:
	; expects ecx zero
	xor edx, edx
noise_loop:
	IMUL ECX, ECX, 0x19660D
	ADD ECX, 0x3C6EF35F
	MOV EAX, ECX
	SHR EAX, 0x12
	MOV [EDX+noise], AL
	INC EDX
	CMP EDX, NOISE_SIZE_BYTES
	JL noise_loop

window_init:
%ifdef FULLSCREEN
	call ChangeDisplaySettingsA
%endif

	call waveOutOpen
	mov ebp, dword [noise]
	call ShowCursor
	call CreateWindowExA
	push eax
	call GetDC
	push eax
	mov edi, eax ; edi is hdc from now on
	call ChoosePixelFormat
	push eax
	push edi
	call SetPixelFormat
	push edi
	call wglCreateContext
	push eax
	push edi
	call wglMakeCurrent
	GLCHECK

gl_proc_loader:
	mov esi, gl_proc_names
	mov ebx, gl_procs
gl_proc_loader_loop:
	push esi
	call wglGetProcAddress
	mov [ebx], eax
	lea ebx, [ebx + 4]
gl_proc_skip_until_zero:
	mov al, [esi]
	inc esi
	test al, al
	jnz gl_proc_skip_until_zero
	cmp [esi], al
	jnz gl_proc_loader_loop
	GLCHECK

alloc_resources:
	call glGenTextures
	GLCHECK

init_textures:
	initTexture tex_noise, NOISE_SIZE, NOISE_SIZE, GL_RGBA, GL_UNSIGNED_BYTE, noise

	push src_main
	push 1
	push GL_FRAGMENT_SHADER
	call glCreateShaderProgramv
	push eax
	call glUseProgram
	GLCHECK

	push GL_BLEND
	call glEnable
	push GL_ONE_MINUS_SRC_ALPHA
	push GL_SRC_ALPHA
	call glBlendFunc

%if 1
	push wavehdr_size
	push wavehdr
	push ebp
	call waveOutWrite
	;CHECK(waveOutWrite(hWaveOut, &WaveHDR, sizeof(WaveHDR)));
%endif

	; ????
	push ebp
	push ebp
	push ebp
	push ebp
mainloop:
	mov ebx, esp
	mov dword [ebx], 4
	; waveOutGetPosition(hWaveOut, &mmtime, sizeof(mmtime))
	push 12
	push ebx
	push ebp
	call waveOutGetPosition
	mov eax, dword [esp + 4]
	cmp eax, MAX_SAMPLES * 8
	jge exit

	push eax
	push F
	push prog_main
	call glGetUniformLocation
	push eax
	call glUniform1i

	push 0
	push S
	push prog_main
	call glGetUniformLocation
	push eax
	call glUniform1i

	push 1
	push 1
	push byte -1
	push byte -1
	call glRects

	push edi
	call SwapBuffers

	push 01bH ;GetAsyncKeyState

	push 1
	push 0
	push 0
	push 0
	push 0
	call PeekMessageA
	call GetAsyncKeyState
	jz mainloop

exit:
	call ExitProcess
