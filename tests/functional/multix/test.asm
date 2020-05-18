; OPTS -c configs/multix.ini

; Test the "TEST" command
; The only proper thing that current MULTIX emulation does here
; is sending IWYTE interrupt

	.include cpu.inc
	.include io.inc
	.include multix.inc

	uj	start

mask:	.word	IMASK_CH0_1

		; 0=ou
		; 1=in	cmd		irq
seq:	.word	-1,	-1,		MX_IWYZE
	.word	0,	MX_CMD_TEST,	MX_IWYTE
seqe:

	.org	OS_START

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

c_ou:	ou	r5, r1 + 1\IO_CHAN
	.word	c_no, c_en, c_ok, c_pe
c_in:	in	r5, r1 + 1\IO_CHAN
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
	rw	r1, INTV_CH1

	lw	r2, seq
	im	mask
loop:	hlt
	ujs	loop

stack:

; XPCT rz[15] : 0
; XPCT rz[6] : 0
; XPCT alarm : 0
; XPCT ir : 0xec3f
