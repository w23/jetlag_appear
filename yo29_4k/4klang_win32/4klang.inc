%macro export_func 1
   global _%1
   _%1:
%endmacro
%define USE_SECTIONS
%define SAMPLE_RATE	44100
%define MAX_INSTRUMENTS	10
%define MAX_VOICES 2
%define HLD 1
%define BPM 60.000000
%define MAX_PATTERNS 57
%define PATTERN_SIZE_SHIFT 4
%define PATTERN_SIZE (1 << PATTERN_SIZE_SHIFT)
%define	MAX_TICKS (MAX_PATTERNS*PATTERN_SIZE)
%define	SAMPLES_PER_TICK 11025
%define DEF_LFO_NORMALIZE 0.0000226757
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
%define 	GO4K_USE_VCO_CHECK
%define 	GO4K_USE_VCO_PHASE_OFFSET
%define 	GO4K_USE_VCO_SHAPE
%define 	GO4K_USE_VCO_MOD_TM
%define 	GO4K_USE_VCO_MOD_DM
%define 	GO4K_USE_VCO_MOD_CM
%define 	GO4K_USE_VCF_CHECK
%define 	GO4K_USE_VCF_MOD_FM
%define 	GO4K_USE_VCF_MOD_RM
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
%define 	GO4K_USE_DLL_MOD_IM
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
	db	39, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, 
	db	HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, 
	db	44, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, 
	db	45, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, 
	db	66, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, 
	db	72, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, 
	db	64, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, 
	db	60, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, 
	db	73, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, 
	db	82, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, 
	db	80, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, 
	db	63, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, 
	db	63, HLD, HLD, HLD, HLD, HLD, HLD, HLD, 0, 0, 0, 0, 0, 0, 0, 0, 
	db	68, HLD, HLD, HLD, HLD, HLD, HLD, HLD, 0, 0, 0, 0, 0, 0, 0, 0, 
	db	57, HLD, HLD, HLD, HLD, HLD, HLD, HLD, 0, 0, 0, 0, 0, 0, 0, 0, 
	db	64, HLD, HLD, HLD, HLD, HLD, HLD, HLD, 0, 0, 0, 0, 0, 0, 0, 0, 
	db	0, 0, 0, 0, 15, HLD, HLD, HLD, HLD, HLD, HLD, HLD, 0, 0, 0, 0, 
	db	0, 0, 0, 0, 27, HLD, HLD, HLD, HLD, HLD, HLD, HLD, 0, 0, 0, 0, 
	db	63, HLD, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	db	68, HLD, 0, 0, 0, 0, 0, 0, 69, HLD, 0, 0, 0, 0, 0, 0, 
	db	60, HLD, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	db	96, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, 
	db	48, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, 
	db	HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, 0, 0, 0, 0, 
	db	70, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, 
	db	68, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, 
	db	0, 0, 0, 0, 0, 0, 0, 0, 82, HLD, HLD, HLD, HLD, HLD, HLD, HLD, 
	db	HLD, HLD, HLD, HLD, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	db	84, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, 
	db	82, HLD, HLD, HLD, HLD, HLD, HLD, HLD, 75, HLD, HLD, HLD, HLD, HLD, HLD, HLD, 
	db	85, HLD, HLD, HLD, HLD, HLD, HLD, HLD, 87, HLD, HLD, HLD, HLD, HLD, HLD, HLD, 
	db	88, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, 
	db	85, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, 0, 0, 0, 0, 
	db	69, HLD, HLD, HLD, HLD, HLD, HLD, HLD, 71, HLD, HLD, HLD, HLD, HLD, HLD, HLD, 
	db	87, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, 
	db	HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, 0, 0, 0, 0, 0, 0, 0, 0, 
	db	69, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, 0, 0, 0, 0, 
	db	68, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, 0, 0, 0, 0, 
	db	68, 71, 75, 78, 80, 75, 78, 71, 68, 71, 75, 78, 80, 75, 78, 71, 
go4k_patterns_end
%ifdef USE_SECTIONS
section .g4kmuc2 data align=1
%else
section .data
%endif
go4k_pattern_lists
Instrument0List		db	1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 1, 2, 2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 0, 0, 0, 0, 0, 
Instrument1List		db	0, 0, 0, 0, 5, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 6, 2, 2, 2, 5, 2, 2, 2, 2, 2, 2, 2, 7, 2, 2, 2, 2, 2, 2, 2, 8, 2, 2, 2, 2, 2, 2, 2, 8, 2, 2, 2, 2, 2, 2, 2, 0, 0, 0, 0, 0, 
Instrument2List		db	0, 0, 0, 0, 0, 0, 0, 0, 9, 2, 2, 2, 6, 2, 2, 2, 10, 2, 11, 2, 9, 2, 2, 2, 6, 2, 2, 2, 9, 2, 2, 2, 2, 2, 2, 2, 10, 2, 2, 2, 12, 2, 2, 2, 10, 2, 2, 2, 12, 2, 2, 2, 2, 2, 2, 2, 0, 
Instrument4List		db	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 13, 0, 13, 0, 14, 14, 14, 14, 13, 13, 13, 13, 13, 13, 13, 13, 15, 15, 15, 15, 16, 16, 16, 16, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 0, 0, 0, 0, 0, 
Instrument6List		db	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 17, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 17, 17, 17, 17, 17, 18, 17, 18, 17, 18, 17, 18, 17, 18, 17, 18, 17, 18, 17, 18, 0, 0, 0, 0, 0, 
Instrument9List		db	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 19, 20, 21, 0, 19, 20, 21, 0, 19, 20, 21, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
Instrument10List		db	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 22, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 23, 2, 0, 0, 0, 
Instrument11List		db	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 24, 25, 24, 26, 2, 27, 28, 11, 2, 29, 30, 9, 31, 32, 33, 26, 34, 9, 0, 35, 24, 29, 0, 10, 2, 11, 0, 0, 0, 0, 0, 6, 36, 37, 38, 0, 0, 0, 0, 0, 
Instrument12List		db	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 39, 39, 39, 39, 39, 39, 39, 39, 0, 0, 0, 0, 0, 
Instrument14List		db	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
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
	db GO4K_VCF_ID
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
	db GO4K_VCF_ID
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
	db GO4K_VCF_ID
	db GO4K_PAN_ID
	db GO4K_OUT_ID
GO4K_END_CMDDEF
GO4K_BEGIN_CMDDEF(Instrument4)
	db GO4K_ENV_ID
	db GO4K_VCO_ID
	db GO4K_FOP_ID
	db GO4K_VCO_ID
	db GO4K_FOP_ID
	db GO4K_VCF_ID
	db GO4K_DLL_ID
	db GO4K_ENV_ID
	db GO4K_FOP_ID
	db GO4K_DLL_ID
	db GO4K_PAN_ID
	db GO4K_OUT_ID
GO4K_END_CMDDEF
GO4K_BEGIN_CMDDEF(Instrument6)
	db GO4K_ENV_ID
	db GO4K_FSTG_ID
	db GO4K_FSTG_ID
	db GO4K_VCO_ID
	db GO4K_FOP_ID
	db GO4K_ENV_ID
	db GO4K_DST_ID
	db GO4K_FST_ID
	db GO4K_FOP_ID
	db GO4K_VCF_ID
	db GO4K_FOP_ID
	db GO4K_FOP_ID
	db GO4K_ENV_ID
	db GO4K_FOP_ID
	db GO4K_PAN_ID
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
	db GO4K_DLL_ID
	db GO4K_OUT_ID
GO4K_END_CMDDEF
GO4K_BEGIN_CMDDEF(Instrument10)
	db GO4K_ENV_ID
	db GO4K_VCO_ID
	db GO4K_FOP_ID
	db GO4K_VCO_ID
	db GO4K_FOP_ID
	db GO4K_DLL_ID
	db GO4K_ENV_ID
	db GO4K_FOP_ID
	db GO4K_PAN_ID
	db GO4K_OUT_ID
	db GO4K_VCO_ID
	db GO4K_DST_ID
	db GO4K_FST_ID
GO4K_END_CMDDEF
GO4K_BEGIN_CMDDEF(Instrument11)
	db GO4K_ENV_ID
	db GO4K_VCO_ID
	db GO4K_FOP_ID
	db GO4K_VCF_ID
	db GO4K_ENV_ID
	db GO4K_FOP_ID
	db GO4K_FST_ID
	db GO4K_VCO_ID
	db GO4K_FST_ID
	db GO4K_FST_ID
	db GO4K_FOP_ID
	db GO4K_FST_ID
	db GO4K_DLL_ID
	db GO4K_VCF_ID
	db GO4K_VCF_ID
	db GO4K_PAN_ID
	db GO4K_OUT_ID
GO4K_END_CMDDEF
GO4K_BEGIN_CMDDEF(Instrument12)
	db GO4K_ENV_ID
	db GO4K_VCO_ID
	db GO4K_VCO_ID
	db GO4K_VCO_ID
	db GO4K_FOP_ID
	db GO4K_FOP_ID
	db GO4K_VCF_ID
	db GO4K_FOP_ID
	db GO4K_ENV_ID
	db GO4K_FOP_ID
	db GO4K_PAN_ID
	db GO4K_FOP_ID
	db GO4K_DLL_ID
	db GO4K_FOP_ID
	db GO4K_ENV_ID
	db GO4K_FOP_ID
	db GO4K_OUT_ID
GO4K_END_CMDDEF
GO4K_BEGIN_CMDDEF(Instrument14)
	db GO4K_ENV_ID
	db GO4K_VCO_ID
	db GO4K_DST_ID
	db GO4K_VCO_ID
	db GO4K_FOP_ID
	db GO4K_FST_ID
	db GO4K_FST_ID
	db GO4K_FOP_ID
	db GO4K_VCO_ID
	db GO4K_VCF_ID
	db GO4K_FOP_ID
	db GO4K_VCF_ID
	db GO4K_FOP_ID
	db GO4K_VCF_ID
	db GO4K_FOP_ID
	db GO4K_FOP_ID
	db GO4K_DLL_ID
	db GO4K_ENV_ID
	db GO4K_FST_ID
	db GO4K_PAN_ID
	db GO4K_OUT_ID
GO4K_END_CMDDEF
GO4K_BEGIN_CMDDEF(Global)
	db GO4K_ACC_ID
	db GO4K_DLL_ID
	db GO4K_FOP_ID
	db GO4K_DLL_ID
	db GO4K_FOP_ID
	db GO4K_ACC_ID
	db GO4K_FOP_ID
	db GO4K_VCO_ID
	db GO4K_FSTG_ID
	db GO4K_FSTG_ID
	db GO4K_FOP_ID
	db GO4K_VCO_ID
	db GO4K_FSTG_ID
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
	GO4K_ENV	ATTAC(96),DECAY(0),SUSTAIN(128),RELEASE(90),GAIN(32)
	GO4K_FST	AMOUNT(128),DEST(15*MAX_UNIT_SLOTS+1+FST_SET)
	GO4K_FST	AMOUNT(96),DEST(15*MAX_UNIT_SLOTS+0+FST_SET)
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
	GO4K_VCF	FREQUENCY(24),RESONANCE(73),VCFTYPE(HIGHPASS)
	GO4K_PAN	PANNING(64)
	GO4K_OUT	GAIN(20), AUXSEND(2)
GO4K_END_PARAMDEF
GO4K_BEGIN_PARAMDEF(Instrument1)
	GO4K_ENV	ATTAC(96),DECAY(0),SUSTAIN(128),RELEASE(90),GAIN(32)
	GO4K_FST	AMOUNT(128),DEST(15*MAX_UNIT_SLOTS+1+FST_SET)
	GO4K_FST	AMOUNT(96),DEST(15*MAX_UNIT_SLOTS+0+FST_SET)
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
	GO4K_VCF	FREQUENCY(24),RESONANCE(73),VCFTYPE(HIGHPASS)
	GO4K_PAN	PANNING(32)
	GO4K_OUT	GAIN(20), AUXSEND(2)
GO4K_END_PARAMDEF
GO4K_BEGIN_PARAMDEF(Instrument2)
	GO4K_ENV	ATTAC(96),DECAY(0),SUSTAIN(128),RELEASE(90),GAIN(32)
	GO4K_FST	AMOUNT(128),DEST(15*MAX_UNIT_SLOTS+1+FST_SET)
	GO4K_FST	AMOUNT(96),DEST(15*MAX_UNIT_SLOTS+0+FST_SET)
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
	GO4K_VCF	FREQUENCY(24),RESONANCE(73),VCFTYPE(HIGHPASS)
	GO4K_PAN	PANNING(96)
	GO4K_OUT	GAIN(20), AUXSEND(2)
GO4K_END_PARAMDEF
GO4K_BEGIN_PARAMDEF(Instrument4)
	GO4K_ENV	ATTAC(32),DECAY(81),SUSTAIN(118),RELEASE(44),GAIN(100)
	GO4K_VCO	TRANSPOSE(40),DETUNE(64),PHASE(0),GATES(0),COLOR(64),SHAPE(87),GAIN(128),FLAGS(SINE)
	GO4K_FOP	OP(FOP_MULP)
	GO4K_VCO	TRANSPOSE(40),DETUNE(60),PHASE(0),GATES(0),COLOR(118),SHAPE(83),GAIN(128),FLAGS(TRISAW)
	GO4K_FOP	OP(FOP_ADDP)
	GO4K_VCF	FREQUENCY(39),RESONANCE(119),VCFTYPE(LOWPASS)
	GO4K_DLL	PREGAIN(42),DRY(128),FEEDBACK(41),DAMP(64),FREQUENCY(0),DEPTH(0),DELAY(17),COUNT(1)
	GO4K_ENV	ATTAC(32),DECAY(84),SUSTAIN(5),RELEASE(60),GAIN(128)
	GO4K_FOP	OP(FOP_MULP)
	GO4K_DLL	PREGAIN(64),DRY(128),FEEDBACK(97),DAMP(21),FREQUENCY(0),DEPTH(16),DELAY(18),COUNT(1)
	GO4K_PAN	PANNING(64)
	GO4K_OUT	GAIN(128), AUXSEND(116)
GO4K_END_PARAMDEF
GO4K_BEGIN_PARAMDEF(Instrument6)
	GO4K_ENV	ATTAC(47),DECAY(64),SUSTAIN(125),RELEASE(104),GAIN(117)
	GO4K_FSTG	AMOUNT(100),DEST((8*go4k_instrument.size*MAX_VOICES/4)+(9*MAX_UNIT_SLOTS+4)+(go4k_instrument.workspace/4)+FST_SET)
	GO4K_FSTG	AMOUNT(0),DEST(7*4+go4k_instrument.workspace)
	GO4K_VCO	TRANSPOSE(64),DETUNE(64),PHASE(0),GATES(0),COLOR(128),SHAPE(64),GAIN(128),FLAGS(SINE)
	GO4K_FOP	OP(FOP_MULP)
	GO4K_ENV	ATTAC(0),DECAY(104),SUSTAIN(0),RELEASE(104),GAIN(128)
	GO4K_DST	DRIVE(12), SNHFREQ(128), FLAGS(0)
	GO4K_FST	AMOUNT(80),DEST(3*MAX_UNIT_SLOTS+1+FST_SET)
	GO4K_FOP	OP(FOP_POP)
	GO4K_VCF	FREQUENCY(10),RESONANCE(123),VCFTYPE(HIGHPASS)
	GO4K_FOP	OP(FOP_PUSH)
	GO4K_FOP	OP(FOP_ADDP)
	GO4K_ENV	ATTAC(31),DECAY(81),SUSTAIN(0),RELEASE(91),GAIN(128)
	GO4K_FOP	OP(FOP_MULP)
	GO4K_PAN	PANNING(64)
	GO4K_OUT	GAIN(128), AUXSEND(73)
GO4K_END_PARAMDEF
GO4K_BEGIN_PARAMDEF(Instrument9)
	GO4K_ENV	ATTAC(32),DECAY(80),SUSTAIN(64),RELEASE(64),GAIN(128)
	GO4K_VCO	TRANSPOSE(88),DETUNE(64),PHASE(0),GATES(85),COLOR(128),SHAPE(64),GAIN(128),FLAGS(SINE)
	GO4K_FOP	OP(FOP_MULP)
	GO4K_PAN	PANNING(64)
	GO4K_DLL	PREGAIN(64),DRY(64),FEEDBACK(64),DAMP(64),FREQUENCY(0),DEPTH(0),DELAY(19),COUNT(1)
	GO4K_FOP	OP(FOP_XCH)
	GO4K_DLL	PREGAIN(64),DRY(64),FEEDBACK(64),DAMP(64),FREQUENCY(0),DEPTH(0),DELAY(20),COUNT(1)
	GO4K_FOP	OP(FOP_XCH)
	GO4K_DLL	PREGAIN(64),DRY(128),FEEDBACK(108),DAMP(64),FREQUENCY(37),DEPTH(8),DELAY(9),COUNT(8)
	GO4K_OUT	GAIN(64), AUXSEND(104)
GO4K_END_PARAMDEF
GO4K_BEGIN_PARAMDEF(Instrument10)
	GO4K_ENV	ATTAC(78),DECAY(94),SUSTAIN(0),RELEASE(64),GAIN(37)
	GO4K_VCO	TRANSPOSE(64),DETUNE(64),PHASE(0),GATES(0),COLOR(53),SHAPE(64),GAIN(128),FLAGS(TRISAW)
	GO4K_FOP	OP(FOP_MULP)
	GO4K_VCO	TRANSPOSE(119),DETUNE(55),PHASE(0),GATES(0),COLOR(127),SHAPE(64),GAIN(128),FLAGS(SINE)
	GO4K_FOP	OP(FOP_MULP)
	GO4K_DLL	PREGAIN(64),DRY(128),FEEDBACK(124),DAMP(0),FREQUENCY(0),DEPTH(0),DELAY(9),COUNT(8)
	GO4K_ENV	ATTAC(31),DECAY(111),SUSTAIN(75),RELEASE(117),GAIN(128)
	GO4K_FOP	OP(FOP_MULP)
	GO4K_PAN	PANNING(30)
	GO4K_OUT	GAIN(68), AUXSEND(28)
	GO4K_VCO	TRANSPOSE(39),DETUNE(64),PHASE(0),GATES(0),COLOR(128),SHAPE(64),GAIN(128),FLAGS(NOISE|LFO)
	GO4K_DST	DRIVE(64), SNHFREQ(2), FLAGS(0)
	GO4K_FST	AMOUNT(69),DEST(3*MAX_UNIT_SLOTS+1+FST_SET+FST_POP)
GO4K_END_PARAMDEF
GO4K_BEGIN_PARAMDEF(Instrument11)
	GO4K_ENV	ATTAC(88),DECAY(90),SUSTAIN(64),RELEASE(90),GAIN(64)
	GO4K_VCO	TRANSPOSE(64),DETUNE(58),PHASE(0),GATES(0),COLOR(64),SHAPE(64),GAIN(112),FLAGS(TRISAW)
	GO4K_FOP	OP(FOP_MULP)
	GO4K_VCF	FREQUENCY(61),RESONANCE(122),VCFTYPE(LOWPASS)
	GO4K_ENV	ATTAC(92),DECAY(78),SUSTAIN(64),RELEASE(90),GAIN(128)
	GO4K_FOP	OP(FOP_ADDP)
	GO4K_FST	AMOUNT(23),DEST(1*MAX_UNIT_SLOTS+5+FST_SET)
	GO4K_VCO	TRANSPOSE(69),DETUNE(64),PHASE(0),GATES(0),COLOR(128),SHAPE(64),GAIN(112),FLAGS(SINE|LFO)
	GO4K_FST	AMOUNT(68),DEST(1*MAX_UNIT_SLOTS+2+FST_SET)
	GO4K_FST	AMOUNT(70),DEST(3*MAX_UNIT_SLOTS+5+FST_SET)
	GO4K_FOP	OP(FOP_POP)
	GO4K_FST	AMOUNT(54),DEST(15*MAX_UNIT_SLOTS+0+FST_SET)
	GO4K_DLL	PREGAIN(64),DRY(96),FEEDBACK(64),DAMP(64),FREQUENCY(0),DEPTH(0),DELAY(-1),COUNT(1)
	GO4K_VCF	FREQUENCY(32),RESONANCE(100),VCFTYPE(HIGHPASS)
	GO4K_VCF	FREQUENCY(54),RESONANCE(64),VCFTYPE(LOWPASS)
	GO4K_PAN	PANNING(64)
	GO4K_OUT	GAIN(32), AUXSEND(70)
GO4K_END_PARAMDEF
GO4K_BEGIN_PARAMDEF(Instrument12)
	GO4K_ENV	ATTAC(31),DECAY(68),SUSTAIN(2),RELEASE(20),GAIN(48)
	GO4K_VCO	TRANSPOSE(64),DETUNE(64),PHASE(0),GATES(0),COLOR(128),SHAPE(66),GAIN(128),FLAGS(TRISAW)
	GO4K_VCO	TRANSPOSE(64),DETUNE(66),PHASE(24),GATES(0),COLOR(128),SHAPE(64),GAIN(128),FLAGS(TRISAW)
	GO4K_VCO	TRANSPOSE(64),DETUNE(61),PHASE(48),GATES(0),COLOR(128),SHAPE(64),GAIN(128),FLAGS(TRISAW)
	GO4K_FOP	OP(FOP_ADDP)
	GO4K_FOP	OP(FOP_ADDP)
	GO4K_VCF	FREQUENCY(31),RESONANCE(114),VCFTYPE(LOWPASS)
	GO4K_FOP	OP(FOP_MULP)
	GO4K_ENV	ATTAC(0),DECAY(88),SUSTAIN(88),RELEASE(88),GAIN(128)
	GO4K_FOP	OP(FOP_POP)
	GO4K_PAN	PANNING(22)
	GO4K_FOP	OP(FOP_XCH)
	GO4K_DLL	PREGAIN(64),DRY(128),FEEDBACK(96),DAMP(88),FREQUENCY(0),DEPTH(0),DELAY(9),COUNT(8)
	GO4K_FOP	OP(FOP_XCH)
	GO4K_ENV	ATTAC(31),DECAY(72),SUSTAIN(0),RELEASE(64),GAIN(128)
	GO4K_FOP	OP(FOP_MULP)
	GO4K_OUT	GAIN(1), AUXSEND(112)
GO4K_END_PARAMDEF
GO4K_BEGIN_PARAMDEF(Instrument14)
	GO4K_ENV	ATTAC(56),DECAY(83),SUSTAIN(64),RELEASE(100),GAIN(32)
	GO4K_VCO	TRANSPOSE(64),DETUNE(64),PHASE(0),GATES(85),COLOR(64),SHAPE(64),GAIN(64),FLAGS(NOISE)
	GO4K_DST	DRIVE(96), SNHFREQ(128), FLAGS(0)
	GO4K_VCO	TRANSPOSE(16),DETUNE(64),PHASE(0),GATES(85),COLOR(128),SHAPE(64),GAIN(87),FLAGS(TRISAW|LFO)
	GO4K_FOP	OP(FOP_ADDP)
	GO4K_FST	AMOUNT(48),DEST(9*MAX_UNIT_SLOTS+4+FST_SET)
	GO4K_FST	AMOUNT(58),DEST(9*MAX_UNIT_SLOTS+5+FST_SET)
	GO4K_FOP	OP(FOP_POP)
	GO4K_VCO	TRANSPOSE(64),DETUNE(64),PHASE(64),GATES(85),COLOR(64),SHAPE(64),GAIN(6),FLAGS(NOISE)
	GO4K_VCF	FREQUENCY(84),RESONANCE(11),VCFTYPE(HIGHPASS)
	GO4K_FOP	OP(FOP_PUSH)
	GO4K_VCF	FREQUENCY(104),RESONANCE(104),VCFTYPE(HIGHPASS)
	GO4K_FOP	OP(FOP_XCH)
	GO4K_VCF	FREQUENCY(86),RESONANCE(112),VCFTYPE(HIGHPASS)
	GO4K_FOP	OP(FOP_ADDP)
	GO4K_FOP	OP(FOP_MULP)
	GO4K_DLL	PREGAIN(32),DRY(128),FEEDBACK(122),DAMP(128),FREQUENCY(39),DEPTH(97),DELAY(9),COUNT(8)
	GO4K_ENV	ATTAC(0),DECAY(96),SUSTAIN(0),RELEASE(128),GAIN(128)
	GO4K_FST	AMOUNT(96),DEST(19*MAX_UNIT_SLOTS+0+FST_SET+FST_POP)
	GO4K_PAN	PANNING(32)
	GO4K_OUT	GAIN(16), AUXSEND(112)
GO4K_END_PARAMDEF
GO4K_BEGIN_PARAMDEF(Global)
	GO4K_ACC	ACCTYPE(AUX)
	GO4K_DLL	PREGAIN(40),DRY(128),FEEDBACK(125),DAMP(64),FREQUENCY(0),DEPTH(0),DELAY(1),COUNT(8)
	GO4K_FOP	OP(FOP_XCH)
	GO4K_DLL	PREGAIN(40),DRY(128),FEEDBACK(125),DAMP(64),FREQUENCY(0),DEPTH(0),DELAY(9),COUNT(8)
	GO4K_FOP	OP(FOP_XCH)
	GO4K_ACC	ACCTYPE(OUTPUT)
	GO4K_FOP	OP(FOP_ADDP2)
	GO4K_VCO	TRANSPOSE(10),DETUNE(64),PHASE(0),GATES(0),COLOR(128),SHAPE(64),GAIN(128),FLAGS(SINE|LFO)
	GO4K_FSTG	AMOUNT(83),DEST((6*go4k_instrument.size*MAX_VOICES/4)+(13*MAX_UNIT_SLOTS+4)+(go4k_instrument.workspace/4)+FST_SET)
	GO4K_FSTG	AMOUNT(80),DEST((5*go4k_instrument.size*MAX_VOICES/4)+(4*MAX_UNIT_SLOTS+2)+(go4k_instrument.workspace/4)+FST_SET)
	GO4K_FOP	OP(FOP_POP)
	GO4K_VCO	TRANSPOSE(36),DETUNE(64),PHASE(0),GATES(0),COLOR(128),SHAPE(64),GAIN(128),FLAGS(SINE)
	GO4K_FSTG	AMOUNT(74),DEST((5*go4k_instrument.size*MAX_VOICES/4)+(10*MAX_UNIT_SLOTS+4)+(go4k_instrument.workspace/4)+FST_SET)
	GO4K_FOP	OP(FOP_POP)
	GO4K_OUT	GAIN(56), AUXSEND(0)
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
	dw 44100
	dw 65535
	dw 144
	dw 160
%endif
