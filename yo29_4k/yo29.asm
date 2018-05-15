BITS 32
global _entrypoint

%define AUDIO_THREAD
%define WIDTH 1280
%define HEIGHT 720
%define NOISE_SIZE 1024
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
%if 0
	DW	028H, 01H
	DD	025H
	DB	00H, 020H, 00H, 00H, 00H, 00H, 00H, 00H, 08H, 00H, 00H, 00H, 00H, 00H
	DB	00H, 020H, 00H, 00H, 00H, 00H
	DD	00H, 00H, 00H
%else
	DW	00H, 00H
	DD	21H ;025H
	DB	00H, 00H, 00H, 00H, 00H, 00H, 00H, 00H, 00H, 00H, 00H, 00H, 00H, 00H
	DB	00H, 00H, 00H, 00H, 00H, 00H
	DD	00H, 00H, 00H
%endif

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
dev_null:
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
	FNCALL glBindTexture, GL_TEXTURE_2D, %1
	FNCALL glTexImage2D, GL_TEXTURE_2D, 0, %4, %2, %3, 0, GL_RGBA, %5, %6
	FNCALL glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR
%endmacro

%ifdef DEBUG
	WNDCLASS EQU static
%else
	%define WNDCLASS SZ_WORD 0xc018
%endif

%define SZ_WORD
%define SZ_BYTE

section .centry text align=1
_entrypoint:
%if 1
	%define ZERO 0
%else
	%define ZERO ecx
	xor ZERO, ZERO
%endif

pushonly:
%macro FNCALL_ARGSONLY 1-*
	%rep %0-1
		%rotate -1
		push %1
	%endrep
%endmacro

%define FNCALL FNCALL_ARGSONLY

	FNCALL_ARGSONLY waveOutWrite, wavehdr, SZ_BYTE wavehdr_size
	FNCALL glCreateShaderProgramv, GL_FRAGMENT_SHADER, 1, src_main
	FNCALL glTexParameteri, SZ_WORD GL_TEXTURE_2D, SZ_WORD GL_TEXTURE_MIN_FILTER, SZ_WORD GL_LINEAR
	FNCALL glTexImage2D, SZ_WORD GL_TEXTURE_2D, ZERO, SZ_WORD GL_RGBA, SZ_WORD NOISE_SIZE, SZ_WORD NOISE_SIZE, ZERO, SZ_WORD GL_RGBA, SZ_WORD GL_UNSIGNED_BYTE, noise
	FNCALL glBindTexture, SZ_WORD GL_TEXTURE_2D, tex_noise
	FNCALL glGenTextures, SZ_BYTE 1, dev_null
	FNCALL glBlendFunc, SZ_WORD GL_SRC_ALPHA, SZ_WORD GL_ONE_MINUS_SRC_ALPHA
	FNCALL glEnable, SZ_WORD GL_BLEND
	FNCALL_ARGSONLY SetPixelFormat, pfd
	FNCALL_ARGSONLY ChoosePixelFormat, pfd
	FNCALL CreateWindowExA, ZERO, WNDCLASS, ZERO, 0x90000000, ZERO, ZERO, SZ_WORD WIDTH, SZ_WORD HEIGHT, ZERO, ZERO, ZERO, ZERO
	FNCALL ShowCursor, ZERO
	FNCALL waveOutOpen, noise, byte -1, wavefmt, ZERO, ZERO, ZERO

;window_init:
%ifdef FULLSCREEN
	FNCALL ChangeDisplaySettingsA, devmode, SZ_BYTE 4
%endif

%ifndef DEBUG ; crashes :(
%ifdef AUDIO_THREAD
;	CreateThread(0, 0, (LPTHREAD_START_ROUTINE)soundRender, sound_buffer, 0, 0);
	FNCALL CreateThread, ZERO, ZERO, __4klang_render@4, sound_buffer, ZERO, ZERO
%else
	FNCALL __4klang_render@4, sound_buffer
%endif
%endif

	; grow stack for waveOutGetPosition mmtime struct
%if 0
	push ebp
	push ebp
	push ebp
	push ebp
%endif

%undef ZERO
%define ZERO 0

%macro DO_INIT 0
%ifndef DEBUG
%ifdef AUDIO_THREAD
;	CreateThread(0, 0, (LPTHREAD_START_ROUTINE)soundRender, sound_buffer, 0, 0);
	FNCALL CreateThread, ZERO, ZERO, __4klang_render@4, sound_buffer, ZERO, ZERO
%else
	FNCALL __4klang_render@4, sound_buffer
%endif
%endif

;window_init:
%ifdef FULLSCREEN
	FNCALL ChangeDisplaySettingsA, devmode, 4
%endif

	;CHECK(waveOutOpen(&hWaveOut, WAVE_MAPPER, &WaveFMT, NULL, 0, CALLBACK_NULL));
	FNCALL waveOutOpen, noise, byte -1, wavefmt, ZERO, ZERO, ZERO
	DO_INST mov, ebp, dword [noise]

	FNCALL ShowCursor, ZERO
	FNCALL CreateWindowExA, ZERO, WNDCLASS, ZERO, 0x90000000, ZERO, ZERO, WIDTH, HEIGHT, ZERO, ZERO, ZERO, ZERO
	DO_INST push, eax
	DO_INST call, GetDC
	FNCALL_ARGSONLY ChoosePixelFormat, pfd
	DO_INST push, eax
	DO_INST mov, edi, eax ; edi is hdc from now on
	DO_INST call, ChoosePixelFormat
	FNCALL_ARGSONLY SetPixelFormat, pfd
	DO_INST push, eax
	DO_INST push, edi
	DO_INST call, SetPixelFormat
	DO_INST push, edi
	DO_INST call, wglCreateContext
	DO_INST push, eax
	DO_INST push, edi
	DO_INST call, wglMakeCurrent
	GLCHECK

	DO_GL_PROC
	DO_GENERATE_NOISE

	FNCALL glEnable, GL_BLEND
	FNCALL glBlendFunc, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA

;alloc_resources:
	FNCALL glGenTextures, 1, dev_null
	GLCHECK

;init_textures:
	initTexture tex_noise, NOISE_SIZE, NOISE_SIZE, GL_RGBA, GL_UNSIGNED_BYTE, noise

	FNCALL glCreateShaderProgramv, GL_FRAGMENT_SHADER, 1, src_main
	DO_INST push, eax
	DO_INST call, glUseProgram
	GLCHECK

	;CHECK(waveOutWrite(hWaveOut, &WaveHDR, sizeof(WaveHDR)));
	FNCALL_ARGSONLY waveOutWrite, wavehdr, wavehdr_size
	DO_INST push, ebp
	DO_INST call, waveOutWrite

	; grow stack for waveOutGetPosition mmtime struct
%if 0
	push ebp
	push ebp
	push ebp
	push ebp
%endif
%endmacro ; DO_INIT


	;DO_INIT
%undef FNCALL_ARGSONLY
%undef FNCALL
%undef DO_GL_PROC
%undef DO_INST

%macro FNCALL_ARGSONLY 1-*
%endmacro

%macro FNCALL 1-*
	call %1
%endmacro

%if 0
%macro FNCALL 1-*
	%rep %0-1
		%rotate -1
		push %1
	%endrep
	%rotate -1
	call %1
%endmacro
%endif

%macro DO_GL_PROC 0
;gl_proc_loader:
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
%endmacro

%macro DO_GENERATE_NOISE 0
;generate_noise:
	;; ecx is not zero after CreateThread; expects ecx zero
	xor edx, edx
	xor ecx, ecx
noise_loop:
	IMUL ECX, ECX, 0x19660D
	ADD ECX, 0x3C6EF35F
	MOV EAX, ECX
	SHR EAX, 0x12
	MOV [EDX+noise], AL
	INC EDX
	CMP EDX, NOISE_SIZE_BYTES
	JL noise_loop
%endmacro

%macro DO_INST 3
	%1 %2, %3
%endmacro

%macro DO_INST 2
	%1 %2
%endmacro

do_only:
	DO_INIT

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

	push 01bH ;GetAsyncKeyState

	push 1
	push 0
	push 0
	push 0
	push 0

	push edi

	push 1
	push 1
	push byte -1
	push byte -1

	push 0
	push S
	push prog_main

	push eax
	push F
	push prog_main
	call glGetUniformLocation
	push eax
	call glUniform1i

	call glGetUniformLocation
	push eax
	call glUniform1i

	call glRects
	call SwapBuffers
	call PeekMessageA
	call GetAsyncKeyState
	jz mainloop

exit:
	call ExitProcess
