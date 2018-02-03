%macro export_func 1
   global _%1
   _%1:
%endmacro
%define SAMPLE_RATE	44100
%define MAX_INSTRUMENTS	12
%define MAX_VOICES 2
%define HLD 1
%define BPM 120.000000
%define MAX_PATTERNS 92
%define PATTERN_SIZE_SHIFT 4
%define PATTERN_SIZE (1 << PATTERN_SIZE_SHIFT)
%define	MAX_TICKS (MAX_PATTERNS*PATTERN_SIZE)
%define	SAMPLES_PER_TICK 5512
%define DEF_LFO_NORMALIZE 0.0000453515
%define	MAX_SAMPLES	(SAMPLES_PER_TICK*MAX_TICKS)
;%define 	GO4K_USE_16BIT_OUTPUT
;%define 	GO4K_USE_GROOVE_PATTERN
;%define 	GO4K_USE_ENVELOPE_RECORDINGS
;%define 	GO4K_USE_NOTE_RECORDINGS
%define 	GO4K_CLIP_OUTPUT
%define 	GO4K_USE_DST
%define 	GO4K_USE_DLL
%define 	GO4K_USE_PAN
%define 	GO4K_USE_GLOBAL_DLL
%define 	GO4K_USE_FSTG
%define 	GO4K_USE_FLD
%define 	GO4K_USE_ENV_CHECK
%define 	GO4K_USE_ENV_MOD_GM
%define 	GO4K_USE_VCO_CHECK
%define 	GO4K_USE_VCO_PHASE_OFFSET
%define 	GO4K_USE_VCO_SHAPE
%define 	GO4K_USE_VCO_MOD_TM
%define 	GO4K_USE_VCF_CHECK
%define 	GO4K_USE_VCF_MOD_FM
%define 	GO4K_USE_VCF_HIGH
%define 	GO4K_USE_VCF_BAND
%define 	GO4K_USE_VCF_PEAK
%define 	GO4K_USE_DST_CHECK
%define 	GO4K_USE_DST_SH
%define		GO4K_USE_DLL_NOTE_SYNC
%define		GO4K_USE_DLL_CHORUS
%define		GO4K_USE_DLL_CHORUS_CLAMP
%define 	GO4K_USE_DLL_DAMP
%define 	GO4K_USE_DLL_DC_FILTER
%define 	GO4K_USE_FSTG_CHECK
%define 	GO4K_USE_PAN_MOD
%define 	GO4K_USE_OUT_MOD_AM
%define 	GO4K_USE_OUT_MOD_GM
%define		GO4K_USE_WAVESHAPER_CLIP
%define 	GO4K_USE_FLD_MOD_VM
%define 	GO4K_USE_DLL_MOD
%define 	GO4K_USE_DLL_MOD_DM
%define	MAX_DELAY			65536
%define MAX_UNITS			48
%define	MAX_UNIT_SLOTS	    9
%define GO4K_BEGIN_CMDDEF(def_name)
%define GO4K_END_CMDDEF db 0
%define GO4K_BEGIN_PARAMDEF(def_name)
%define GO4K_END_PARAMDEF
GO4K_ENV_ID		equ		1
%macro	GO4K_ENV 5
	db	%1
	db	%2
	db	%3
	db	%4
	db	%5
%endmacro
%define	ATTAC(val)		val	
%define	DECAY(val)		val	
%define	SUSTAIN(val)	val	
%define	RELEASE(val)	val	
%define	GAIN(val)		val	
struc	go4kENV_val
	.attac		resd	1
	.decay		resd	1
	.sustain	resd	1
	.release	resd	1
	.gain		resd	1
	.size
endstruc
struc	go4kENV_wrk
	.state		resd	1
	.level		resd	1
	.gm			resd	1
	.am			resd	1
	.dm			resd	1
	.sm			resd	1
	.rm			resd	1
	.size
endstruc
%define ENV_STATE_ATTAC		0
%define ENV_STATE_DECAY		1
%define ENV_STATE_SUSTAIN	2
%define ENV_STATE_RELEASE	3
%define ENV_STATE_OFF		4
GO4K_VCO_ID		equ		2
%macro	GO4K_VCO 8
	db	%1
	db	%2
%ifdef GO4K_USE_VCO_PHASE_OFFSET	
	db	%3
%endif	
%ifdef GO4K_USE_VCO_GATE	
	db	%4
%endif	
	db	%5
%ifdef GO4K_USE_VCO_SHAPE	
	db	%6
%endif	
	db	%7
	db	%8
%endmacro
%define	TRANSPOSE(val)	val	
%define	DETUNE(val)		val	
%define	PHASE(val)		val	
%define	GATES(val)		val	
%define	COLOR(val)		val	
%define	SHAPE(val)		val 
%define	FLAGS(val)		val	
%define SINE		0x01
%define TRISAW		0x02
%define PULSE		0x04
%define NOISE		0x08
%define LFO			0x10
%define GATE		0x20
%define	VCO_STEREO	0x40
struc	go4kVCO_val
	.transpose	resd	1
	.detune		resd	1
%ifdef GO4K_USE_VCO_PHASE_OFFSET	
	.phaseofs	resd	1
%endif	
%ifdef GO4K_USE_VCO_GATE	
	.gate		resd	1
%endif	
	.color		resd	1
%ifdef GO4K_USE_VCO_SHAPE	
	.shape		resd	1
%endif	
	.gain		resd	1
	.flags		resd	1	
	.size
endstruc
struc	go4kVCO_wrk
	.phase		resd	1
	.tm			resd	1
	.dm			resd	1
	.fm			resd	1
	.pm			resd	1
	.cm			resd	1
	.sm			resd	1
	.gm			resd	1
	.phase2		resd	1
	.size
endstruc
GO4K_VCF_ID		equ		3
%macro	GO4K_VCF 3
	db	%1
	db	%2
	db	%3
%endmacro
%define LOWPASS		0x1
%define HIGHPASS	0x2
%define BANDPASS	0x4
%define	BANDSTOP	0x3
%define ALLPASS		0x7
%define	PEAK		0x8
%define STEREO		0x10
%define	FREQUENCY(val)	val
%define	RESONANCE(val)	val
%define	VCFTYPE(val)	val
struc	go4kVCF_val
	.freq		resd	1
	.res		resd	1
	.type		resd	1
	.size
endstruc
struc	go4kVCF_wrk
	.low		resd	1
	.high		resd	1
	.band		resd	1
	.freq		resd	1
	.fm			resd	1
	.rm			resd	1
	.low2		resd	1
	.high2		resd	1
	.band2		resd	1
	.size
endstruc
GO4K_DST_ID		equ		4
%macro	GO4K_DST 3
	db	%1
%ifdef GO4K_USE_DST_SH
	db	%2
%ifdef GO4K_USE_DST_STEREO
	db	%3
%endif
%else
%ifdef GO4K_USE_DST_STEREO
	db	%3
%endif
%endif
%endmacro
%define	DRIVE(val)		val
%define	SNHFREQ(val)	val
%define	FLAGS(val)		val
struc	go4kDST_val
	.drive		resd	1
%ifdef GO4K_USE_DST_SH	
	.snhfreq	resd	1
%endif	
	.flags		resd	1
	.size
endstruc
struc	go4kDST_wrk
	.out		resd	1
	.snhphase	resd	1
	.dm			resd	1
	.sm			resd	1
	.out2		resd	1
	.size
endstruc
GO4K_DLL_ID		equ		5
%macro	GO4K_DLL 8
	db	%1
	db	%2
	db	%3
%ifdef GO4K_USE_DLL_DAMP	
	db	%4
%endif		
%ifdef GO4K_USE_DLL_CHORUS	
	db	%5
	db	%6
%endif
	db	%7
	db	%8
%endmacro
%define PREGAIN(val)	val
%define	DRY(val)		val
%define	FEEDBACK(val)	val
%define	DEPTH(val)		val
%define DAMP(val)		val
%define	DELAY(val)		val
%define	COUNT(val)		val
struc	go4kDLL_val
	.pregain	resd	1
	.dry		resd	1
	.feedback	resd	1
%ifdef GO4K_USE_DLL_DAMP	
	.damp		resd	1	
%endif
%ifdef GO4K_USE_DLL_CHORUS
	.freq		resd	1
	.depth
%endif
	.delay		resd	1
	.count		resd	1
	.size
endstruc
struc	go4kDLL_wrk
	.index		resd	1
	.store		resd	1
	.dcin		resd	1
	.dcout		resd	1
%ifdef GO4K_USE_DLL_CHORUS
	.phase		resd	1
%endif
	.buffer		resd	MAX_DELAY
	.size
endstruc
struc	go4kDLL_wrk2
 .pm			resd	1
 .fm			resd	1
 .im			resd	1
 .dm			resd	1
 .sm			resd	1
 .am			resd	1
 .size
endstruc
GO4K_FOP_ID	equ			6
%macro	GO4K_FOP 1
	db	%1
%endmacro
%define	OP(val)			val
%define FOP_POP			0x1
%define FOP_ADDP		0x2
%define FOP_MULP		0x3
%define FOP_PUSH		0x4
%define FOP_XCH			0x5
%define FOP_ADD			0x6
%define FOP_MUL			0x7
%define FOP_ADDP2		0x8
%define FOP_LOADNOTE	0x9
%define FOP_MULP2		0xa
struc	go4kFOP_val
	.flags		resd	1
	.size
endstruc
struc	go4kFOP_wrk
	.size
endstruc
GO4K_FST_ID		equ		7
%macro	GO4K_FST 2
	db	%1
	dw	%2
%endmacro
%define	AMOUNT(val)		val
%define	DEST(val)		val
%define	FST_SET			0x0000
%define	FST_ADD			0x4000
%define	FST_POP			0x8000
struc	go4kFST_val
	.amount		resd	1
	.op1		resd	1
	.size
endstruc
struc	go4kFST_wrk
	.size
endstruc
GO4K_PAN_ID		equ		8
%macro	GO4K_PAN 1
%ifdef GO4K_USE_PAN
	db	%1
%endif
%endmacro
%define	PANNING(val)	val
struc	go4kPAN_val
%ifdef GO4K_USE_PAN
	.panning	resd	1
%endif
	.size
endstruc
struc	go4kPAN_wrk
	.pm			resd	1
	.size
endstruc
GO4K_OUT_ID		equ		9
%macro	GO4K_OUT 2
	db	%1
%ifdef GO4K_USE_GLOBAL_DLL	
	db	%2
%endif	
%endmacro
%define	AUXSEND(val)	val
struc	go4kOUT_val
	.gain		resd	1
%ifdef GO4K_USE_GLOBAL_DLL	
	.auxsend	resd	1
%endif
	.size
endstruc
struc	go4kOUT_wrk
	.am			resd	1
	.gm			resd	1
	.size
endstruc
GO4K_ACC_ID		equ		10
%macro	GO4K_ACC 1
	db	%1
%endmacro
%define OUTPUT			0
%define	AUX				8
%define ACCTYPE(val)	val
struc	go4kACC_val
	.acctype	resd	1
	.size
endstruc
struc	go4kACC_wrk
	.size
endstruc
%ifdef GO4K_USE_FLD
GO4K_FLD_ID	equ		11
%macro	GO4K_FLD 1
	db	%1
%endmacro
%define	VALUE(val)	val
struc	go4kFLD_val
	.value		resd	1
	.size
endstruc
struc	go4kFLD_wrk
	.vm			resd	1
	.size
endstruc
%endif
%ifdef GO4K_USE_FSTG
GO4K_FSTG_ID	equ		12
%macro	GO4K_FSTG 2
	db	%1
	dw	%2
%endmacro
struc	go4kFSTG_val
	.amount		resd	1
	.op1		resd	1
	.size
endstruc
struc	go4kFSTG_wrk
	.size
endstruc
%endif
struc	go4k_instrument
	.release	resd	1
	.note		resd	1
	.workspace	resd	MAX_UNITS*MAX_UNIT_SLOTS
	.dlloutl	resd	1
	.dlloutr	resd	1
	.outl		resd	1
	.outr		resd	1
	.size
endstruc
struc	go4k_synth
	.instruments	resb	go4k_instrument.size * MAX_INSTRUMENTS * MAX_VOICES
	.global			resb	go4k_instrument.size * MAX_VOICES
	.size
endstruc
%ifdef USE_SECTIONS
section .g4kmuc1 data align=1
%else
section .data align=1
%endif
go4k_patterns
	db	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	db	74, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, 
	db	HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, 
	db	HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, 0, 0, 0, 0, 0, 0, 0, 0, 
	db	67, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, 
	db	68, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, 
	db	70, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, 
	db	77, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, 
	db	63, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, 
	db	61, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, 
	db	65, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, 
	db	62, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, 
	db	60, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, 
	db	71, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, 
	db	0, 0, 0, 0, 0, 0, 0, 0, 71, HLD, HLD, HLD, HLD, HLD, HLD, HLD, 
	db	0, 0, 0, 0, 0, 0, 0, 0, 84, HLD, HLD, HLD, HLD, HLD, HLD, HLD, 
	db	0, 0, 84, HLD, 0, 84, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	db	0, 0, 0, 0, 0, 0, 84, HLD, 0, 0, 0, 0, 0, 0, 0, 0, 
	db	0, 0, 0, 0, 113, HLD, 0, 0, 0, 0, 0, 0, 65, HLD, 53, 0, 
	db	0, 0, 0, 0, 91, HLD, 0, 0, 0, 0, 0, 0, 65, HLD, 53, 0, 
	db	0, 0, 0, 0, 0, 0, 0, 0, 72, HLD, HLD, HLD, HLD, HLD, HLD, HLD, 
	db	HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, 108, HLD, HLD, HLD, HLD, HLD, HLD, HLD, 
	db	HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, 72, HLD, HLD, HLD, HLD, HLD, HLD, HLD, 
	db	HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, 72, HLD, 0, 0, 0, 0, 0, 0, 
	db	48, HLD, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 48, HLD, 
	db	48, HLD, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	db	86, HLD, 84, HLD, 74, HLD, 0, 0, 81, HLD, 79, HLD, 69, HLD, 74, HLD, 
	db	67, 0, 0, 0, 0, 0, 51, HLD, 0, 0, 0, 0, 0, 0, 0, 0, 
	db	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 74, HLD, 0, 0, 0, 0, 
	db	70, HLD, 0, 0, 0, 0, 50, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	db	73, 0, 0, 0, 0, 0, 72, HLD, 0, 0, 0, 0, 0, 0, 0, 0, 
	db	70, HLD, 0, 0, 0, 0, 0, 0, 0, 0, 73, HLD, 0, 0, 0, 0, 
	db	68, HLD, 0, 0, 0, 0, 53, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	db	67, HLD, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	db	68, 0, 0, 0, 0, 0, 70, HLD, 0, 0, 0, 0, 0, 0, 0, 0, 
	db	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 77, HLD, 0, 0, 0, 0, 
	db	70, HLD, 0, 0, 0, 0, 67, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	db	79, 0, 0, 0, 0, 0, 77, HLD, 0, 0, 0, 0, 0, 0, 0, 0, 
	db	74, HLD, 0, 0, 0, 0, 72, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	db	71, HLD, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	db	0, 0, 0, 0, 0, 0, 60, HLD, 0, 0, 0, 0, 0, 0, 0, 0, 
	db	0, 0, 0, 0, 0, 0, 53, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	db	0, 0, 0, 0, 0, 0, 56, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	db	0, 0, 0, 0, 0, 0, 71, HLD, 0, 0, 0, 0, 0, 0, 0, 0, 
	db	0, 0, 0, 0, 0, 0, 70, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	db	0, 0, 0, 0, 0, 0, 80, HLD, 0, 0, 0, 0, 0, 0, 0, 0, 
	db	0, 0, 0, 0, 0, 0, 74, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	db	0, 0, 0, 0, 0, 0, 62, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	db	0, 0, 0, 0, 0, 0, 58, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	db	0, 0, 0, 0, 0, 0, 61, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	db	0, 0, 0, 0, 0, 0, 60, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	db	0, 0, 0, 0, 0, 0, 75, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	db	0, 0, 0, 0, 0, 0, 84, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	db	0, 0, 0, 0, 0, 0, 79, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	db	0, 0, 0, 0, 50, HLD, 0, 0, 0, 0, 50, HLD, HLD, HLD, 0, 0, 
	db	0, 0, 0, 0, 50, HLD, HLD, HLD, 0, 0, 0, 0, 0, 0, 0, 0, 
	db	0, 0, 0, 0, 56, HLD, 0, 0, 0, 0, 56, HLD, HLD, HLD, 0, 0, 
	db	0, 0, 0, 0, 56, HLD, HLD, HLD, 0, 0, 0, 0, 0, 0, 0, 0, 
	db	0, 0, 0, 0, 55, HLD, 0, 0, 0, 0, 55, HLD, HLD, HLD, 0, 0, 
	db	0, 0, 0, 0, 55, HLD, HLD, HLD, 0, 0, 0, 0, 0, 0, 0, 0, 
	db	0, 0, 0, 0, 57, HLD, 0, 0, 0, 0, 57, HLD, HLD, HLD, 0, 0, 
	db	0, 0, 0, 0, 57, HLD, HLD, HLD, 0, 0, 0, 0, 0, 0, 0, 0, 
	db	0, 0, 0, 0, 53, HLD, 0, 0, 0, 0, 53, HLD, HLD, HLD, 0, 0, 
	db	0, 0, 0, 0, 53, HLD, HLD, HLD, 0, 0, 0, 0, 0, 0, 0, 0, 
	db	0, 0, 0, 0, 51, HLD, 0, 0, 0, 0, 51, HLD, HLD, HLD, 0, 0, 
	db	0, 0, 0, 0, 51, HLD, HLD, HLD, 0, 0, 0, 0, 0, 0, 0, 0, 
	db	0, 0, 0, 0, 49, HLD, 0, 0, 0, 0, 49, HLD, HLD, HLD, 0, 0, 
	db	0, 0, 0, 0, 49, HLD, HLD, HLD, 0, 0, 0, 0, 0, 0, 0, 0, 
go4k_patterns_end
%ifdef USE_SECTIONS
section .g4kmuc2 data align=1
%else
section .data
%endif
go4k_pattern_lists
Instrument0List		db	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 2, 3, 0, 0, 0, 0, 4, 2, 2, 3, 0, 0, 0, 0, 1, 2, 2, 3, 0, 0, 0, 0, 4, 2, 2, 3, 0, 0, 0, 0, 5, 3, 4, 3, 6, 3, 7, 3, 5, 3, 4, 3, 6, 3, 7, 3, 5, 3, 8, 3, 9, 3, 4, 3, 4, 2, 2, 3, 0, 0, 0, 0, 
Instrument1List		db	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10, 2, 2, 3, 0, 0, 0, 0, 11, 2, 2, 3, 0, 0, 0, 0, 10, 2, 2, 3, 0, 0, 0, 0, 11, 2, 2, 3, 0, 0, 0, 0, 12, 3, 10, 3, 9, 3, 12, 3, 12, 3, 10, 3, 9, 3, 12, 3, 13, 3, 6, 3, 5, 3, 14, 3, 13, 2, 2, 3, 0, 0, 0, 0, 
Instrument2List		db	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 2, 2, 3, 0, 0, 0, 0, 13, 2, 2, 3, 0, 0, 0, 0, 4, 2, 2, 3, 0, 0, 0, 0, 13, 2, 2, 3, 0, 0, 0, 0, 8, 3, 11, 3, 10, 3, 5, 3, 8, 3, 11, 3, 10, 3, 5, 3, 8, 3, 4, 3, 10, 3, 1, 3, 1, 2, 2, 3, 0, 0, 0, 0, 
Instrument3List		db	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 15, 2, 2, 3, 0, 0, 1, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
Instrument4List		db	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 0, 0, 0, 16, 0, 0, 0, 16, 0, 0, 0, 16, 0, 0, 0, 16, 0, 0, 0, 16, 0, 0, 0, 16, 0, 0, 0, 16, 0, 0, 0, 0, 0, 0, 0, 
Instrument5List		db	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 17, 18, 17, 19, 17, 18, 17, 19, 17, 18, 17, 19, 17, 18, 17, 19, 17, 18, 17, 19, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 17, 0, 0, 0, 17, 0, 0, 0, 17, 0, 0, 0, 17, 0, 0, 0, 17, 0, 0, 0, 17, 0, 0, 0, 17, 0, 0, 0, 0, 0, 0, 0, 
Instrument6List		db	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 20, 21, 22, 21, 22, 21, 22, 21, 22, 21, 22, 21, 22, 21, 22, 21, 22, 21, 22, 21, 22, 21, 22, 21, 22, 21, 22, 21, 23, 0, 0, 0, 20, 21, 22, 21, 22, 21, 22, 21, 22, 21, 22, 21, 22, 21, 22, 21, 22, 21, 22, 21, 22, 21, 22, 21, 23, 0, 0, 0, 0, 0, 0, 0, 
Instrument7List		db	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 24, 25, 25, 0, 24, 25, 25, 0, 24, 25, 25, 0, 24, 25, 25, 0, 24, 25, 25, 0, 24, 25, 25, 0, 24, 25, 25, 0, 24, 25, 25, 0, 24, 25, 25, 0, 24, 25, 25, 0, 24, 25, 25, 0, 24, 25, 25, 0, 24, 25, 25, 0, 24, 25, 25, 0, 24, 25, 25, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
Instrument8List		db	26, 0, 0, 0, 26, 0, 0, 0, 26, 0, 0, 0, 26, 0, 0, 0, 26, 0, 26, 0, 26, 0, 26, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 27, 28, 29, 0, 30, 31, 32, 33, 27, 28, 29, 0, 30, 31, 32, 33, 34, 35, 36, 0, 37, 35, 38, 39, 0, 0, 0, 0, 0, 0, 0, 0, 
Instrument9List		db	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 40, 0, 41, 0, 41, 0, 42, 0, 40, 0, 41, 0, 41, 0, 42, 0, 43, 0, 44, 0, 45, 0, 46, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
Instrument10List		db	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 47, 0, 48, 0, 49, 0, 50, 0, 47, 0, 48, 0, 49, 0, 50, 0, 51, 0, 51, 0, 52, 0, 53, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
Instrument12List		db	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 54, 55, 54, 55, 54, 55, 54, 55, 54, 55, 54, 55, 54, 55, 54, 55, 54, 55, 54, 55, 54, 55, 54, 55, 54, 55, 54, 55, 54, 55, 54, 55, 54, 55, 54, 55, 0, 0, 0, 0, 56, 57, 58, 59, 60, 61, 62, 63, 56, 57, 58, 59, 60, 61, 62, 63, 56, 57, 64, 65, 66, 67, 58, 59, 0, 0, 0, 0, 0, 0, 0, 0, 
go4k_pattern_lists_end
%ifdef USE_SECTIONS
section .g4kmuc3 data align=1
%else
section .data
%endif
go4k_synth_instructions
GO4K_BEGIN_CMDDEF(Instrument0)
	db GO4K_ENV_ID
	db GO4K_FST_ID
	db GO4K_FST_ID
	db GO4K_FST_ID
	db GO4K_VCO_ID
	db GO4K_VCF_ID
	db GO4K_FST_ID
	db GO4K_DLL_ID
	db GO4K_VCO_ID
	db GO4K_FOP_ID
	db GO4K_FLD_ID
	db GO4K_FOP_ID
	db GO4K_DLL_ID
	db GO4K_PAN_ID
	db GO4K_OUT_ID
GO4K_END_CMDDEF
GO4K_BEGIN_CMDDEF(Instrument1)
	db GO4K_ENV_ID
	db GO4K_FST_ID
	db GO4K_FST_ID
	db GO4K_FST_ID
	db GO4K_VCO_ID
	db GO4K_VCF_ID
	db GO4K_FST_ID
	db GO4K_DLL_ID
	db GO4K_VCO_ID
	db GO4K_FOP_ID
	db GO4K_FLD_ID
	db GO4K_FOP_ID
	db GO4K_DLL_ID
	db GO4K_PAN_ID
	db GO4K_OUT_ID
GO4K_END_CMDDEF
GO4K_BEGIN_CMDDEF(Instrument2)
	db GO4K_ENV_ID
	db GO4K_FST_ID
	db GO4K_FST_ID
	db GO4K_FST_ID
	db GO4K_VCO_ID
	db GO4K_VCF_ID
	db GO4K_FST_ID
	db GO4K_DLL_ID
	db GO4K_VCO_ID
	db GO4K_FOP_ID
	db GO4K_FLD_ID
	db GO4K_FOP_ID
	db GO4K_DLL_ID
	db GO4K_PAN_ID
	db GO4K_OUT_ID
GO4K_END_CMDDEF
GO4K_BEGIN_CMDDEF(Instrument3)
	db GO4K_ENV_ID
	db GO4K_FST_ID
	db GO4K_FST_ID
	db GO4K_FST_ID
	db GO4K_VCO_ID
	db GO4K_VCF_ID
	db GO4K_FST_ID
	db GO4K_DLL_ID
	db GO4K_VCO_ID
	db GO4K_FOP_ID
	db GO4K_FLD_ID
	db GO4K_FOP_ID
	db GO4K_DLL_ID
	db GO4K_PAN_ID
	db GO4K_OUT_ID
GO4K_END_CMDDEF
GO4K_BEGIN_CMDDEF(Instrument4)
	db GO4K_ENV_ID
	db GO4K_VCO_ID
	db GO4K_FOP_ID
	db GO4K_ENV_ID
	db GO4K_FST_ID
	db GO4K_VCF_ID
	db GO4K_VCO_ID
	db GO4K_FST_ID
	db GO4K_DLL_ID
	db GO4K_DLL_ID
	db GO4K_VCF_ID
	db GO4K_PAN_ID
	db GO4K_OUT_ID
	db GO4K_FOP_ID
	db GO4K_FSTG_ID
GO4K_END_CMDDEF
GO4K_BEGIN_CMDDEF(Instrument5)
	db GO4K_ENV_ID
	db GO4K_FST_ID
	db GO4K_ENV_ID
	db GO4K_DST_ID
	db GO4K_FST_ID
	db GO4K_FOP_ID
	db GO4K_VCO_ID
	db GO4K_FOP_ID
	db GO4K_FOP_ID
	db GO4K_ENV_ID
	db GO4K_FOP_ID
	db GO4K_FST_ID
	db GO4K_VCF_ID
	db GO4K_ENV_ID
	db GO4K_FOP_ID
	db GO4K_PAN_ID
	db GO4K_OUT_ID
GO4K_END_CMDDEF
GO4K_BEGIN_CMDDEF(Instrument6)
	db GO4K_ENV_ID
	db GO4K_FST_ID
	db GO4K_ENV_ID
	db GO4K_FST_ID
	db GO4K_FST_ID
	db GO4K_FOP_ID
	db GO4K_VCO_ID
	db GO4K_VCO_ID
	db GO4K_VCO_ID
	db GO4K_VCF_ID
	db GO4K_FOP_ID
	db GO4K_FOP_ID
	db GO4K_FOP_ID
	db GO4K_VCF_ID
	db GO4K_DLL_ID
	db GO4K_VCF_ID
	db GO4K_PAN_ID
	db GO4K_OUT_ID
GO4K_END_CMDDEF
GO4K_BEGIN_CMDDEF(Instrument7)
	db GO4K_ENV_ID
	db GO4K_FST_ID
	db GO4K_ENV_ID
	db GO4K_DST_ID
	db GO4K_FST_ID
	db GO4K_FOP_ID
	db GO4K_VCO_ID
	db GO4K_FOP_ID
	db GO4K_PAN_ID
	db GO4K_OUT_ID
GO4K_END_CMDDEF
GO4K_BEGIN_CMDDEF(Instrument8)
	db GO4K_ENV_ID
	db GO4K_VCO_ID
	db GO4K_FOP_ID
	db GO4K_PAN_ID
	db GO4K_DLL_ID
	db GO4K_FOP_ID
	db GO4K_DLL_ID
	db GO4K_FOP_ID
	db GO4K_OUT_ID
GO4K_END_CMDDEF
GO4K_BEGIN_CMDDEF(Instrument9)
	db GO4K_ENV_ID
	db GO4K_VCO_ID
	db GO4K_FOP_ID
	db GO4K_PAN_ID
	db GO4K_DLL_ID
	db GO4K_FOP_ID
	db GO4K_DLL_ID
	db GO4K_FOP_ID
	db GO4K_OUT_ID
GO4K_END_CMDDEF
GO4K_BEGIN_CMDDEF(Instrument10)
	db GO4K_ENV_ID
	db GO4K_VCO_ID
	db GO4K_FOP_ID
	db GO4K_PAN_ID
	db GO4K_DLL_ID
	db GO4K_FOP_ID
	db GO4K_DLL_ID
	db GO4K_FOP_ID
	db GO4K_OUT_ID
GO4K_END_CMDDEF
GO4K_BEGIN_CMDDEF(Instrument12)
	db GO4K_ENV_ID
	db GO4K_FOP_ID
	db GO4K_ENV_ID
	db GO4K_VCO_ID
	db GO4K_FOP_ID
	db GO4K_FOP_ID
	db GO4K_VCO_ID
	db GO4K_FOP_ID
	db GO4K_FOP_ID
	db GO4K_FST_ID
	db GO4K_PAN_ID
	db GO4K_OUT_ID
GO4K_END_CMDDEF
GO4K_BEGIN_CMDDEF(Global)
	db GO4K_ACC_ID
	db GO4K_VCF_ID
	db GO4K_DLL_ID
	db GO4K_DLL_ID
	db GO4K_FOP_ID
	db GO4K_VCF_ID
	db GO4K_DLL_ID
	db GO4K_DLL_ID
	db GO4K_ACC_ID
	db GO4K_FOP_ID
	db GO4K_OUT_ID
GO4K_END_CMDDEF
go4k_synth_instructions_end
%ifdef USE_SECTIONS
section .g4kmuc4 data align=1
%else
section .data
%endif
go4k_synth_parameter_values
GO4K_BEGIN_PARAMDEF(Instrument0)
	GO4K_ENV	ATTAC(92),DECAY(0),SUSTAIN(128),RELEASE(92),GAIN(63)
	GO4K_FST	AMOUNT(128),DEST(14*MAX_UNIT_SLOTS+1+FST_SET)
	GO4K_FST	AMOUNT(96),DEST(14*MAX_UNIT_SLOTS+0+FST_SET)
	GO4K_FST	AMOUNT(48),DEST(7*MAX_UNIT_SLOTS+3+FST_SET+FST_POP)
	GO4K_VCO	TRANSPOSE(64),DETUNE(64),PHASE(0),GATES(0),COLOR(64),SHAPE(64),GAIN(128),FLAGS(NOISE)
	GO4K_VCF	FREQUENCY(29),RESONANCE(128),VCFTYPE(LOWPASS)
	GO4K_FST	AMOUNT(32),DEST(10*MAX_UNIT_SLOTS+0+FST_SET)
	GO4K_DLL	PREGAIN(128),DRY(128),FEEDBACK(127),DAMP(122),FREQUENCY(0),DEPTH(3),DELAY(0),COUNT(1)
	GO4K_VCO	TRANSPOSE(64),DETUNE(56),PHASE(0),GATES(0),COLOR(128),SHAPE(64),GAIN(72),FLAGS(SINE)
	GO4K_FOP	OP(FOP_ADDP)
	GO4K_FLD	VALUE(64)
	GO4K_FOP	OP(FOP_ADDP)
	GO4K_DLL	PREGAIN(64),DRY(51),FEEDBACK(100),DAMP(106),FREQUENCY(26),DEPTH(79),DELAY(9),COUNT(8)
	GO4K_PAN	PANNING(96)
	GO4K_OUT	GAIN(0), AUXSEND(0)
GO4K_END_PARAMDEF
GO4K_BEGIN_PARAMDEF(Instrument1)
	GO4K_ENV	ATTAC(92),DECAY(0),SUSTAIN(128),RELEASE(92),GAIN(63)
	GO4K_FST	AMOUNT(128),DEST(14*MAX_UNIT_SLOTS+1+FST_SET)
	GO4K_FST	AMOUNT(96),DEST(14*MAX_UNIT_SLOTS+0+FST_SET)
	GO4K_FST	AMOUNT(48),DEST(7*MAX_UNIT_SLOTS+3+FST_SET+FST_POP)
	GO4K_VCO	TRANSPOSE(64),DETUNE(64),PHASE(0),GATES(0),COLOR(64),SHAPE(64),GAIN(128),FLAGS(NOISE)
	GO4K_VCF	FREQUENCY(29),RESONANCE(128),VCFTYPE(LOWPASS)
	GO4K_FST	AMOUNT(32),DEST(10*MAX_UNIT_SLOTS+0+FST_SET)
	GO4K_DLL	PREGAIN(128),DRY(128),FEEDBACK(127),DAMP(122),FREQUENCY(0),DEPTH(3),DELAY(0),COUNT(1)
	GO4K_VCO	TRANSPOSE(64),DETUNE(56),PHASE(0),GATES(0),COLOR(128),SHAPE(64),GAIN(72),FLAGS(SINE)
	GO4K_FOP	OP(FOP_ADDP)
	GO4K_FLD	VALUE(64)
	GO4K_FOP	OP(FOP_ADDP)
	GO4K_DLL	PREGAIN(64),DRY(51),FEEDBACK(100),DAMP(106),FREQUENCY(26),DEPTH(79),DELAY(9),COUNT(8)
	GO4K_PAN	PANNING(96)
	GO4K_OUT	GAIN(0), AUXSEND(0)
GO4K_END_PARAMDEF
GO4K_BEGIN_PARAMDEF(Instrument2)
	GO4K_ENV	ATTAC(92),DECAY(0),SUSTAIN(128),RELEASE(92),GAIN(63)
	GO4K_FST	AMOUNT(128),DEST(14*MAX_UNIT_SLOTS+1+FST_SET)
	GO4K_FST	AMOUNT(96),DEST(14*MAX_UNIT_SLOTS+0+FST_SET)
	GO4K_FST	AMOUNT(48),DEST(7*MAX_UNIT_SLOTS+3+FST_SET+FST_POP)
	GO4K_VCO	TRANSPOSE(64),DETUNE(64),PHASE(0),GATES(0),COLOR(64),SHAPE(64),GAIN(128),FLAGS(NOISE)
	GO4K_VCF	FREQUENCY(29),RESONANCE(128),VCFTYPE(LOWPASS)
	GO4K_FST	AMOUNT(32),DEST(10*MAX_UNIT_SLOTS+0+FST_SET)
	GO4K_DLL	PREGAIN(128),DRY(128),FEEDBACK(127),DAMP(122),FREQUENCY(0),DEPTH(3),DELAY(0),COUNT(1)
	GO4K_VCO	TRANSPOSE(64),DETUNE(56),PHASE(0),GATES(0),COLOR(128),SHAPE(64),GAIN(72),FLAGS(SINE)
	GO4K_FOP	OP(FOP_ADDP)
	GO4K_FLD	VALUE(64)
	GO4K_FOP	OP(FOP_ADDP)
	GO4K_DLL	PREGAIN(64),DRY(51),FEEDBACK(100),DAMP(106),FREQUENCY(26),DEPTH(79),DELAY(9),COUNT(8)
	GO4K_PAN	PANNING(96)
	GO4K_OUT	GAIN(0), AUXSEND(0)
GO4K_END_PARAMDEF
GO4K_BEGIN_PARAMDEF(Instrument3)
	GO4K_ENV	ATTAC(92),DECAY(0),SUSTAIN(128),RELEASE(92),GAIN(63)
	GO4K_FST	AMOUNT(128),DEST(14*MAX_UNIT_SLOTS+1+FST_SET)
	GO4K_FST	AMOUNT(96),DEST(14*MAX_UNIT_SLOTS+0+FST_SET)
	GO4K_FST	AMOUNT(48),DEST(7*MAX_UNIT_SLOTS+3+FST_SET+FST_POP)
	GO4K_VCO	TRANSPOSE(64),DETUNE(64),PHASE(0),GATES(0),COLOR(64),SHAPE(64),GAIN(128),FLAGS(NOISE)
	GO4K_VCF	FREQUENCY(29),RESONANCE(128),VCFTYPE(LOWPASS)
	GO4K_FST	AMOUNT(32),DEST(10*MAX_UNIT_SLOTS+0+FST_SET)
	GO4K_DLL	PREGAIN(128),DRY(128),FEEDBACK(127),DAMP(122),FREQUENCY(0),DEPTH(3),DELAY(0),COUNT(1)
	GO4K_VCO	TRANSPOSE(64),DETUNE(56),PHASE(0),GATES(0),COLOR(128),SHAPE(64),GAIN(72),FLAGS(SINE)
	GO4K_FOP	OP(FOP_ADDP)
	GO4K_FLD	VALUE(64)
	GO4K_FOP	OP(FOP_ADDP)
	GO4K_DLL	PREGAIN(64),DRY(51),FEEDBACK(100),DAMP(106),FREQUENCY(26),DEPTH(79),DELAY(9),COUNT(8)
	GO4K_PAN	PANNING(96)
	GO4K_OUT	GAIN(0), AUXSEND(0)
GO4K_END_PARAMDEF
GO4K_BEGIN_PARAMDEF(Instrument4)
	GO4K_ENV	ATTAC(0),DECAY(64),SUSTAIN(64),RELEASE(64),GAIN(128)
	GO4K_VCO	TRANSPOSE(128),DETUNE(64),PHASE(0),GATES(0),COLOR(0),SHAPE(64),GAIN(128),FLAGS(TRISAW)
	GO4K_FOP	OP(FOP_MULP)
	GO4K_ENV	ATTAC(66),DECAY(52),SUSTAIN(0),RELEASE(96),GAIN(128)
	GO4K_FST	AMOUNT(28),DEST(5*MAX_UNIT_SLOTS+4+FST_SET+FST_POP)
	GO4K_VCF	FREQUENCY(64),RESONANCE(64),VCFTYPE(LOWPASS)
	GO4K_VCO	TRANSPOSE(0),DETUNE(64),PHASE(0),GATES(0),COLOR(0),SHAPE(1),GAIN(128),FLAGS(NOISE|LFO)
	GO4K_FST	AMOUNT(119),DEST(1*MAX_UNIT_SLOTS+1+FST_SET+FST_POP)
	GO4K_DLL	PREGAIN(64),DRY(128),FEEDBACK(64),DAMP(64),FREQUENCY(0),DEPTH(0),DELAY(17),COUNT(1)
	GO4K_DLL	PREGAIN(6),DRY(128),FEEDBACK(89),DAMP(117),FREQUENCY(124),DEPTH(61),DELAY(9),COUNT(8)
	GO4K_VCF	FREQUENCY(91),RESONANCE(1),VCFTYPE(LOWPASS)
	GO4K_PAN	PANNING(64)
	GO4K_OUT	GAIN(124), AUXSEND(0)
	GO4K_FOP	OP(FOP_LOADNOTE)
	GO4K_FSTG	AMOUNT(96),DEST((5*go4k_instrument.size*MAX_VOICES/4)+(12*MAX_UNIT_SLOTS+4)+(go4k_instrument.workspace/4)+FST_SET+FST_POP)
GO4K_END_PARAMDEF
GO4K_BEGIN_PARAMDEF(Instrument5)
	GO4K_ENV	ATTAC(0),DECAY(80),SUSTAIN(75),RELEASE(77),GAIN(128)
	GO4K_FST	AMOUNT(128),DEST(0*MAX_UNIT_SLOTS+2+FST_SET)
	GO4K_ENV	ATTAC(0),DECAY(64),SUSTAIN(41),RELEASE(106),GAIN(128)
	GO4K_DST	DRIVE(32), SNHFREQ(75), FLAGS(0)
	GO4K_FST	AMOUNT(77),DEST(6*MAX_UNIT_SLOTS+1+FST_SET)
	GO4K_FOP	OP(FOP_POP)
	GO4K_VCO	TRANSPOSE(45),DETUNE(64),PHASE(32),GATES(85),COLOR(128),SHAPE(66),GAIN(128),FLAGS(SINE)
	GO4K_FOP	OP(FOP_MULP)
	GO4K_FOP	OP(FOP_LOADNOTE)
	GO4K_ENV	ATTAC(0),DECAY(87),SUSTAIN(0),RELEASE(0),GAIN(128)
	GO4K_FOP	OP(FOP_MULP)
	GO4K_FST	AMOUNT(90),DEST(12*MAX_UNIT_SLOTS+4+FST_SET+FST_POP)
	GO4K_VCF	FREQUENCY(7),RESONANCE(9),VCFTYPE(HIGHPASS)
	GO4K_ENV	ATTAC(0),DECAY(91),SUSTAIN(0),RELEASE(77),GAIN(128)
	GO4K_FOP	OP(FOP_MULP)
	GO4K_PAN	PANNING(64)
	GO4K_OUT	GAIN(5), AUXSEND(1)
GO4K_END_PARAMDEF
GO4K_BEGIN_PARAMDEF(Instrument6)
	GO4K_ENV	ATTAC(0),DECAY(75),SUSTAIN(0),RELEASE(94),GAIN(54)
	GO4K_FST	AMOUNT(128),DEST(0*MAX_UNIT_SLOTS+2+FST_SET)
	GO4K_ENV	ATTAC(0),DECAY(56),SUSTAIN(0),RELEASE(0),GAIN(128)
	GO4K_FST	AMOUNT(108),DEST(6*MAX_UNIT_SLOTS+1+FST_SET)
	GO4K_FST	AMOUNT(72),DEST(7*MAX_UNIT_SLOTS+1+FST_SET)
	GO4K_FOP	OP(FOP_POP)
	GO4K_VCO	TRANSPOSE(32),DETUNE(64),PHASE(0),GATES(85),COLOR(128),SHAPE(32),GAIN(64),FLAGS(NOISE)
	GO4K_VCO	TRANSPOSE(64),DETUNE(64),PHASE(0),GATES(85),COLOR(128),SHAPE(80),GAIN(64),FLAGS(NOISE)
	GO4K_VCO	TRANSPOSE(64),DETUNE(64),PHASE(0),GATES(85),COLOR(64),SHAPE(64),GAIN(64),FLAGS(NOISE)
	GO4K_VCF	FREQUENCY(57),RESONANCE(68),VCFTYPE(ALLPASS)
	GO4K_FOP	OP(FOP_ADDP)
	GO4K_FOP	OP(FOP_ADDP)
	GO4K_FOP	OP(FOP_MULP)
	GO4K_VCF	FREQUENCY(83),RESONANCE(24),VCFTYPE(HIGHPASS)
	GO4K_DLL	PREGAIN(65),DRY(128),FEEDBACK(64),DAMP(82),FREQUENCY(84),DEPTH(69),DELAY(18),COUNT(1)
	GO4K_VCF	FREQUENCY(67),RESONANCE(9),VCFTYPE(LOWPASS)
	GO4K_PAN	PANNING(64)
	GO4K_OUT	GAIN(41), AUXSEND(17)
GO4K_END_PARAMDEF
GO4K_BEGIN_PARAMDEF(Instrument7)
	GO4K_ENV	ATTAC(0),DECAY(32),SUSTAIN(96),RELEASE(76),GAIN(128)
	GO4K_FST	AMOUNT(128),DEST(0*MAX_UNIT_SLOTS+2+FST_SET)
	GO4K_ENV	ATTAC(0),DECAY(70),SUSTAIN(30),RELEASE(64),GAIN(128)
	GO4K_DST	DRIVE(32), SNHFREQ(128), FLAGS(0)
	GO4K_FST	AMOUNT(80),DEST(6*MAX_UNIT_SLOTS+1+FST_SET)
	GO4K_FOP	OP(FOP_POP)
	GO4K_VCO	TRANSPOSE(45),DETUNE(64),PHASE(0),GATES(85),COLOR(128),SHAPE(66),GAIN(128),FLAGS(SINE)
	GO4K_FOP	OP(FOP_MULP)
	GO4K_PAN	PANNING(64)
	GO4K_OUT	GAIN(128), AUXSEND(0)
GO4K_END_PARAMDEF
GO4K_BEGIN_PARAMDEF(Instrument8)
	GO4K_ENV	ATTAC(0),DECAY(68),SUSTAIN(41),RELEASE(83),GAIN(128)
	GO4K_VCO	TRANSPOSE(88),DETUNE(64),PHASE(0),GATES(85),COLOR(128),SHAPE(64),GAIN(128),FLAGS(SINE)
	GO4K_FOP	OP(FOP_MULP)
	GO4K_PAN	PANNING(64)
	GO4K_DLL	PREGAIN(96),DRY(64),FEEDBACK(64),DAMP(64),FREQUENCY(0),DEPTH(0),DELAY(17),COUNT(1)
	GO4K_FOP	OP(FOP_XCH)
	GO4K_DLL	PREGAIN(96),DRY(64),FEEDBACK(96),DAMP(64),FREQUENCY(0),DEPTH(0),DELAY(19),COUNT(1)
	GO4K_FOP	OP(FOP_XCH)
	GO4K_OUT	GAIN(100), AUXSEND(74)
GO4K_END_PARAMDEF
GO4K_BEGIN_PARAMDEF(Instrument9)
	GO4K_ENV	ATTAC(0),DECAY(68),SUSTAIN(41),RELEASE(83),GAIN(128)
	GO4K_VCO	TRANSPOSE(88),DETUNE(64),PHASE(0),GATES(85),COLOR(128),SHAPE(64),GAIN(128),FLAGS(SINE)
	GO4K_FOP	OP(FOP_MULP)
	GO4K_PAN	PANNING(64)
	GO4K_DLL	PREGAIN(96),DRY(64),FEEDBACK(64),DAMP(64),FREQUENCY(0),DEPTH(0),DELAY(17),COUNT(1)
	GO4K_FOP	OP(FOP_XCH)
	GO4K_DLL	PREGAIN(96),DRY(64),FEEDBACK(96),DAMP(64),FREQUENCY(0),DEPTH(0),DELAY(19),COUNT(1)
	GO4K_FOP	OP(FOP_XCH)
	GO4K_OUT	GAIN(100), AUXSEND(74)
GO4K_END_PARAMDEF
GO4K_BEGIN_PARAMDEF(Instrument10)
	GO4K_ENV	ATTAC(0),DECAY(68),SUSTAIN(41),RELEASE(83),GAIN(128)
	GO4K_VCO	TRANSPOSE(88),DETUNE(64),PHASE(0),GATES(85),COLOR(128),SHAPE(64),GAIN(128),FLAGS(SINE)
	GO4K_FOP	OP(FOP_MULP)
	GO4K_PAN	PANNING(64)
	GO4K_DLL	PREGAIN(96),DRY(64),FEEDBACK(64),DAMP(64),FREQUENCY(0),DEPTH(0),DELAY(17),COUNT(1)
	GO4K_FOP	OP(FOP_XCH)
	GO4K_DLL	PREGAIN(96),DRY(64),FEEDBACK(96),DAMP(64),FREQUENCY(0),DEPTH(0),DELAY(19),COUNT(1)
	GO4K_FOP	OP(FOP_XCH)
	GO4K_OUT	GAIN(100), AUXSEND(74)
GO4K_END_PARAMDEF
GO4K_BEGIN_PARAMDEF(Instrument12)
	GO4K_ENV	ATTAC(30),DECAY(80),SUSTAIN(128),RELEASE(64),GAIN(128)
	GO4K_FOP	OP(FOP_PUSH)
	GO4K_ENV	ATTAC(11),DECAY(80),SUSTAIN(128),RELEASE(64),GAIN(128)
	GO4K_VCO	TRANSPOSE(52),DETUNE(64),PHASE(30),GATES(0),COLOR(64),SHAPE(64),GAIN(128),FLAGS(TRISAW)
	GO4K_FOP	OP(FOP_MULP)
	GO4K_FOP	OP(FOP_ADDP)
	GO4K_VCO	TRANSPOSE(52),DETUNE(69),PHASE(0),GATES(0),COLOR(128),SHAPE(64),GAIN(128),FLAGS(SINE)
	GO4K_FOP	OP(FOP_ADDP)
	GO4K_FOP	OP(FOP_MULP)
	GO4K_FST	AMOUNT(58),DEST(10*MAX_UNIT_SLOTS+0+FST_SET)
	GO4K_PAN	PANNING(94)
	GO4K_OUT	GAIN(128), AUXSEND(16)
GO4K_END_PARAMDEF
GO4K_BEGIN_PARAMDEF(Global)
	GO4K_ACC	ACCTYPE(AUX)
	GO4K_VCF	FREQUENCY(16),RESONANCE(128),VCFTYPE(HIGHPASS)
	GO4K_DLL	PREGAIN(84),DRY(128),FEEDBACK(64),DAMP(64),FREQUENCY(0),DEPTH(0),DELAY(17),COUNT(1)
	GO4K_DLL	PREGAIN(40),DRY(0),FEEDBACK(125),DAMP(48),FREQUENCY(0),DEPTH(0),DELAY(1),COUNT(8)
	GO4K_FOP	OP(FOP_XCH)
	GO4K_VCF	FREQUENCY(16),RESONANCE(128),VCFTYPE(HIGHPASS)
	GO4K_DLL	PREGAIN(84),DRY(128),FEEDBACK(64),DAMP(64),FREQUENCY(0),DEPTH(0),DELAY(20),COUNT(1)
	GO4K_DLL	PREGAIN(40),DRY(0),FEEDBACK(125),DAMP(48),FREQUENCY(0),DEPTH(0),DELAY(9),COUNT(8)
	GO4K_ACC	ACCTYPE(OUTPUT)
	GO4K_FOP	OP(FOP_ADDP2)
	GO4K_OUT	GAIN(32), AUXSEND(0)
GO4K_END_PARAMDEF
go4k_synth_parameter_values_end
%ifdef USE_SECTIONS
section .g4kmuc5 data align=1
%else
section .data
%endif
%ifdef GO4K_USE_DLL
global _go4k_delay_times
_go4k_delay_times
	dw 0
	dw 1116
	dw 1188
	dw 1276
	dw 1356
	dw 1422
	dw 1492
	dw 1556
	dw 1618
	dw 1140
	dw 1212
	dw 1300
	dw 1380
	dw 1446
	dw 1516
	dw 1580
	dw 1642
	dw 22050
	dw 1837
	dw 11025
	dw 14700
%endif
