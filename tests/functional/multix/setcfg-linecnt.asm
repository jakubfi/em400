; OPTS -c configs/multix.ini

	.cpu	mx16

	.include cpu.inc
	.include io.inc
	.include multix.inc

	mcl
	uj	start

msk_mx:	.word	IMASK_CH0_1

	.org	OS_START

; ---------------------------------------------------------------
; line count tests
; ---------------------------------------------------------------

dummy:	.word	0, 0 ; dummy to acknowledge IWYZE
num0:	.word	33\7 + 1\15, 0 ; wrong number of physical line descriptions (>32)
num1:	.word	0\7 + 1\15, 0 ; wrong number of physical line descriptions (=0)
num2:	.word	1\7 + 33\15, 0 ; wrong number of logical lines (>32)
num3:	.word	1\7 + 0\15, 0 ; wrong number of logical lines (=0)
num4:	.word	2\7 + 1\15, 0
	.word	MX_LDIR_FD + MX_LINE_UNUSED + MX_LTYPE_USARTA + 30 ; 31 phy lines
	.word	MX_LDIR_FD + MX_LINE_UNUSED + MX_LTYPE_USARTA + 1 ; +2 phy lines = exceeds number of total 32 physical lines

	.const	test_size 3

seq:
	.word	dummy,	MX_IWYZE,	0
	.word	num0,	MX_INKON,	MX_SCERR_NUM
	.word	num1,	MX_INKON,	MX_SCERR_NUM
	.word	num2,	MX_INKON,	MX_SCERR_NUM
	.word	num3,	MX_INKON,	MX_SCERR_NUM
	.word	num4,	MX_INKON,	MX_SCERR_NUM
seqe:

; ------------------------------------------------------------------------
; expects:
;  r1 - configuration field address
;  r4 - RJ return adress
setcfg:
	ou	r1, MX_CMD_SETCFG | 1\IO_CHAN
	.word	no, en, ok, pe
no:	hlt	041
ok:	uj	r4
en:	ujs	setcfg
pe:	hlt	042

; ------------------------------------------------------------------------
mx_proc:
	lw	r1, [STACKP]
	lw	r1, [r1-1]	; intspec from mx

	cw	r1, [r2+1]	; is intspec as expected?
	bb	r0, ?E
	hlt	040		; bad intspec

	cw	r1, MX_INKOT	; skip setconf result if INKOT
	jes	next

	lw	r1, [r2+2]	; load expected setconf return value
	lw	r3, [r2]
	lw	r5, [r3+1]
	cw	r1, r5		; load setconf return value
	bb	r0, ?E
	hlt	043		; bad setconf result

next:
	awt	r2, test_size	; next test
	cw	r2, seqe	; all tests finished?
	blc	?E
	hlt	077		; all tests OK

	lw	r1, [r2]
	rj	r4, setcfg

	lip

; ------------------------------------------------------------------------
start:
	lw	r1, stack
	rw	r1, STACKP
	lw	r1, mx_proc
	rw	r1, INTV_CH1

	lw	r2, seq		; start of all tests
	im	msk_mx
loop:
	hlt
	ujs	loop

stack:

; XPCT rz[15] : 0
; XPCT rz[6] : 0
; XPCT alarm : 0
; XPCT ir : 0xec3f
