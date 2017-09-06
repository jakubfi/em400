; OPTS -c configs/multix.cfg

; Test handling of illegal commands

	.include hw.inc
	.include mx.inc

	uj	start

mask:	.word	IMASK_CH0_1

		; 0=ou
		; 1=in	cmd		irq
seq:	.word	-1,	-1,		MX_IWYZE
	.word	0,	0b000\2,	MX_IEPS0
	.word	0,	0b110\2,	MX_IEPS6
	.word	0,	0b111\2,	MX_IEPS7
	.word	1,	0b000\2 + 3\4,	MX_IEPS8
	.word	1,	0b100\2,	MX_IEPSC
	.word	1,	0b101\2,	MX_IEPSD
	.word	1,	0b110\2,	MX_IEPSE
	.word	1,	0b111\2,	MX_IEPSF
seqe:

	.org	OS_MEM_BEG

; ------------------------------------------------------------------------
mx_proc:
	lw	r1, [STACKP]
	lw	r1, [r1-1]	; intspec from mx
	cw	r1, [r2+2]	; expected intspec
	bb	r0, ?E
	hlt	040		; bad intspec

	awt	r2, 3		; next test

	; all tests finished?
	cw	r2, seqe
	blc	?E
	hlt	077		; tests OK

	lw	r1, [r2+1]	; load command
	lw	r3, [r2]	; load in/ou option
	lw	r4, c_in
	cw	r3, 1
	bb	r0, ?E		; in?
	lw	r4, c_ou
	uj	r4

c_ou:	ou	r5, r1 + MX_CHAN
	.word	c_no, c_en, c_ok, c_pe
c_in:	in	r5, r1 + MX_CHAN
	.word	c_no, c_en, c_ok, c_pe
c_no:
c_pe:	hlt	041
c_en:	uj	r4	; repeat if engaged
c_ok:	lip

; ------------------------------------------------------------------------
start:
	lw	r1, stack
	rw	r1, STACKP
	lw	r1, mx_proc
	rw	r1, MX_IV

	lw	r2, seq
	im	mask
loop:	hlt
	ujs	loop

stack:

; XPCT rz[15] : 0
; XPCT rz[6] : 0
; XPCT alarm : 0
; XPCT ir : 0xec3f
