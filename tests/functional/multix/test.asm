; OPTS -c configs/multix.cfg

; Test the "TEST" command
; The only proper thing that current MULTIX emulation does here
; is sending IWYTE interrupt

	.const	IWYZE 2
	.const	IWYTE 3

	.equ	stackp 0x61
	.equ	prog_beg 0x70
	.equ	int_mx 0x40 + 12 + 1
	.equ	unmask_chan 0b0000010000000000
	.equ	mx_chan 1

	UJ	start

mask:	.word	unmask_chan

		; 0=ou
		; 1=in	cmd		irq
seq:	.word	-1,	-1,		IWYZE
	.word	0,	0b001\2,	IWYTE
seqe:

	.org	prog_beg

; ------------------------------------------------------------------------
mx_proc:
	LW	r1, [stackp]
	LW	r1, [r1-1]	; intspec from mx
	SHC	r1, 8
	CW	r1, [r2+2]	; expected intspec
	BB	r0, ?E
	HLT	040		; bad intspec

	AWT	r2, 3		; next test

	; all tests finished?
	CW	r2, seqe
	BLC	?E
	HLT	077		; tests OK

	LW	r1, [r2+1]	; load command
	LW	r3, [r2]	; load in/ou option
	LW	r4, c_in
	CW	r3, 1
	BB	r0, ?E		; in?
	LW	r4, c_ou
	UJ	r4

c_ou:	OU	r5, r1 + mx_chan\14
	.word	c_no, c_en, c_ok, c_pe
c_in:	IN	r5, r1 + mx_chan\14
	.word	c_no, c_en, c_ok, c_pe
c_no:
c_pe:	HLT	041
c_en:	UJ	r4	; repeat if engaged
c_ok:	LIP

; ------------------------------------------------------------------------
start:
	LW	r1, stack
	RW	r1, stackp
	LW	r1, mx_proc
	RW	r1, int_mx

	LW	r2, seq
	IM	mask
loop:	HLT
	UJS	loop

stack:

; XPCT rz[15] : 0
; XPCT rz[6] : 0
; XPCT alarm : 0
; XPCT ir&0x3f : 0o77
