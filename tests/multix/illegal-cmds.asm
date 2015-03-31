; OPTS -c configs/multix.cfg

; Test handling of illegal commands

	.const	IWYZE 2
	.const	IEPS0 35
	.const	IEPS6 36
	.const	IEPS7 37
	.const	IEPS8 38
	.const	IEPSC 39
	.const	IEPSD 40
	.const	IEPSE 41
	.const	IEPSF 42

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
	.word	0,	0b000\2,	IEPS0
	.word	0,	0b110\2,	IEPS6
	.word	0,	0b111\2,	IEPS7
	.word	1,	0b000\2 + 3\4,	IEPS8
	.word	1,	0b100\2,	IEPSC
	.word	1,	0b101\2,	IEPSD
	.word	1,	0b110\2,	IEPSE
	.word	1,	0b111\2,	IEPSF
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

; XPCT int(rz[15]) : 0
; XPCT int(rz[6]) : 0
; XPCT int(alarm) : 0
; XPCT oct(ir[10-15]) : 077
