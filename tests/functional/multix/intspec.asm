; OPTS -c configs/multix.cfg

; INTSPEC should:
;  * always return OK
;  * give 0 before initialization
;  * give IWYZE after initialization
;  * give IWYZE when executed 'by hand' after initialization with interrupts disabled
;  * give 0 afterwards

	.equ	stackp 0x61
	.equ	prog_beg 0x70
	.equ	int_mx 0x40 + 12 + 1
	.equ	int_tim 0x40 + 5
	.equ	mx_chan 1
	.equ	iwyze 0b0000001000000000

	UJ	start

cmask:	.word	0b0000010000000000
tmask:	.word	0b0000100000000000

	.org	prog_beg

; ------------------------------------------------------------------------
mx_proc:
	LW	r4, [stackp]
	LW	r4, [r4-1]
	CW	r4, iwyze       ; is it IWYZE?
	BB	r0, ?E
	HLT	050		; not iwyze
	LIP

; ------------------------------------------------------------------------
tim_proc:
	AWT	r7, 1
	LIP

; ------------------------------------------------------------------------
get_intspec:
	LWT	r5, -13
	IN	r5, 0b01\4 + mx_chan\14
	.word	f, f, ok, f
f:	UJ	r3
ok:	CW	r5, r2
	BB	r0, ?E
	UJ	r3
	UJ	r3+1
	
; ------------------------------------------------------------------------
start:
	LW	r3, stack
	RW	r3, stackp
	LW	r3, mx_proc
	RW	r3, int_mx
	LW	r3, tim_proc
	RW	r3, int_tim

	LWT	r7, 0	; reset wait timer counter

	; "intspec" before initialization = 0
	LW	r2, 0
	RJ	r3, get_intspec
	HLT	041

	IM	cmask
	HLT		; wait for mx int

	; "intspec" after acknowledged initialization = 0
	LW	r2, 0
	RJ	r3, get_intspec
	HLT	042

	; reset multix
	IN	r2, 0b00\4 + mx_chan\14
	.word	f3, f3, ok3, f3
f3:	HLT	043
ok3:

	; wait 200ms
	IM	tmask	; disable channel interrupts, enable timer
loop:	HLT
	CW	r7, 30	; wait 30x10ms for multix to initialize
	BB	r0, ?E
	UJS	loop

	; "intspec" after not acknowledged initialization = IWYZE
	LW	r2, iwyze
	RJ	r3, get_intspec
	HLT	044

	; "intspec" after "manually" acknowledged initialization = 0
	LW	r2, 0
	RJ	r3, get_intspec
	HLT	045

	HLT	077

stack:

; XPCT rz[15] : 0
; XPCT rz[6] : 0
; XPCT alarm : 0
; XPCT ir&0x3f : 0o77
