.686
.model flat, stdcall
.stack 4096

externdef C ExitProcess@4:proc
externdef C ShowCursor@4:proc
externdef C CreateWindowExA@48:proc
externdef C GetDC@4:proc
externdef C ChoosePixelFormat@8:proc
externdef C SetPixelFormat@12:proc
externdef C wglCreateContext@4:proc
externdef C wglMakeCurrent@8:proc
externdef C wglGetProcAddress@4:proc
externdef C timeGetTime@0:proc
externdef C glClearColor@16:proc
externdef C glClear@4:proc
externdef C SwapBuffers@4:proc
externdef C PeekMessageA@20:proc
externdef C GetAsyncKeyState@4:proc

;FUNC_OFFSETS

WIDTH_	EQU 1280
HEIGHT	EQU 720

.data
IFDEF FUNC_OFFSETS
	ExitProcess dd ExitProcess@4
	ShowCursor dd ShowCursor@4
	CreateWindowExA dd CreateWindowExA@48
	GetDC dd GetDC@4
	ChoosePixelFormat dd ChoosePixelFormat@8
	SetPixelFormat dd SetPixelFormat@12
	wglCreateContext dd wglCreateContext@4
	wglMakeCurrent dd wglMakeCurrent@8
	wglGetProcAddress dd wglGetProcAddress@4
	timeGetTime dd timeGetTime@0
	glClearColor dd glClearColor@16
	glClear dd glClear@4
	SwapBuffers dd SwapBuffers@4
	PeekMessageA dd PeekMessageA@20
	GetAsyncKeyState dd GetAsyncKeyState@4
ELSE
	ExitProcess EQU ExitProcess@4
	ShowCursor EQU ShowCursor@4
	CreateWindowExA EQU CreateWindowExA@48
	GetDC EQU GetDC@4
	ChoosePixelFormat EQU ChoosePixelFormat@8
	SetPixelFormat EQU SetPixelFormat@12
	wglCreateContext EQU wglCreateContext@4
	wglMakeCurrent EQU wglMakeCurrent@8
	wglGetProcAddress EQU wglGetProcAddress@4
	timeGetTime EQU timeGetTime@0
	glClearColor EQU glClearColor@16
	glClear EQU glClear@4
	SwapBuffers EQU SwapBuffers@4
	PeekMessageA EQU PeekMessageA@20
	GetAsyncKeyState EQU GetAsyncKeyState@4
ENDIF

fcall MACRO func
IFDEF FUNC_OFFSETS
	call dword ptr [ebp + (func - ExitProcess)]
ELSE
	call func
ENDIF
endm


pixelFormatDescriptor	DW	028H
	DW	01H
	DD	025H
	DB	00H
	DB	020H
	DB	00H
	DB	00H
	DB	00H
	DB	00H
	DB	00H
	DB	00H
	DB	08H
	DB	00H
	DB	00H
	DB	00H
	DB	00H
	DB	00H
	DB	00H
	DB	020H
	DB	00H
	DB	00H
	DB	00H
	DB	00H
	DD	00H
	DD	00H
	DD	00H
screenSettings DB 00H
	DW	00H
	DW	00H
	DW	09cH
	DW	00H
	DD	01c0000H
	DW	00H
	DW	00H
	DW	00H
	DW	00H
	DW	00H
	DW	00H
	DB	00H
	DW	00H
	DD	020H
	DD	WIDTH_
	DD	HEIGHT
	DD	00H
	DD	00H
	DD	00H
	DD	00H
	DD	00H
	DD	00H
	DD	00H
	DD	00H
	DD	00H
	DD	00H

	static db "static", 0

IFDEF SEPARATE_STACK
	ShowCursor_args	dd 0
	CreateWindowExA_args dd 0
		;dd 0C018H
		dd static
		dd 0
		dd 90000000H
		dd 0, 0
		dd WIDTH_
		dd HEIGHT
		dd 0, 0, 0, 0
	ChoosePixelFormat_args dd pixelFormatDescriptor
	SetPixelFormat_args dd pixelFormatDescriptor
	Args_end db ?
ELSE
ENDIF

.data?
;bss_begin: dd ?

.code
entrypoint proc
	;mov ebp, OFFSET ExitProcess
IFDEF SEPARATE_STACK
	mov ecx, Args_end - ShowCursor_args
	sub esp, ecx
	mov edi, esp
	mov esi, OFFSET ShowCursor_args
	rep movsb
ELSE
	xor eax, eax
	mov ebx, WIDTH_
	mov ecx, HEIGHT
;	mov edx, OFFSET static
; SetPixelFormat
	push OFFSET pixelFormatDescriptor
; ChoosePixelFormat
	push OFFSET pixelFormatDescriptor
; CreateWindowExA
	push eax
	push eax
	push eax
	push eax
	push ecx
	push ebx
	push eax
	push eax
	push 090000000H
	push eax
	push OFFSET static
	push eax
; ShowCursor
	push eax
ENDIF

	fcall ShowCursor
	fcall CreateWindowExA
	push eax
	fcall GetDC
	push eax
	push eax
	pop ebx
	fcall ChoosePixelFormat
	push eax
	push ebx
	fcall SetPixelFormat
	push ebx
	push ebx
	fcall wglCreateContext
	push eax
	push ebx
	fcall wglMakeCurrent

	; TODO wglGetProcAddress

	fcall timeGetTime
	push ebx
	mov ebx, eax

mainloop:
	fcall timeGetTime
	sub eax, ebx
	push eax
	fild dword ptr [esp]
	mov dword ptr [esp], 1000
	fidiv dword ptr [esp]
	fsin
	fstp dword ptr [esp]
	pop eax

	push eax
	push eax
	push eax
	push eax
	fcall glClearColor
	push 000004000H
	fcall glClear

	pop ebx
	push ebx
	push ebx
	fcall SwapBuffers
	push 01bH
	fcall GetAsyncKeyState
	jz mainloop

exit:	
	fcall ExitProcess
entrypoint endp
end
