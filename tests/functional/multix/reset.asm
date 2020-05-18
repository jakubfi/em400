; OPTS -c configs/multix.ini

; Check if all MULTIX reset triggers work

	.include cpu.inc
	.include io.inc
	.include multix.inc

	uj	start

mask0:	.word	IMASK_NONE
mask:	.word	IMASK_CH0_1

; ------------------------------------------------------------------------
mx_proc:
	lw	r4, [STACKP]
	lw	r4, [r4-1]
	cw	r4, MX_IWYZE
	jn	int_fail
	aw	r3, 1		; increase interrupt counter
	or	r6, ?X		; set MX int. indicator
tim_proc:
	lip
int_fail:
	im	mask0
	hlt	041

; ------------------------------------------------------------------------
	.org	OS_START
start:
	lw	r3, stack
	rw	r3, STACKP

	lw	r3, mx_proc
	rw	r3, INTV_CH1

	lw	r3, tim_proc
	rw	r3, INTV_TIMER
	rw	r3, INTV_UNUSED

	lwt	r3, 0		; reset interrupt counter
	er	r6, ?X		; reset MX int. indicator

	; IWYZE by MULTIX initialization
	im	mask
w1:	hlt
	bb	r6, ?X
	ujs	w1
	er	r6, ?X

	; IWYZE by MCL
	mcl
	im	mask
w2:	hlt
	bb	r6, ?X
	ujs	w2
	er	r6, ?X

	; IWYZE, by software MULTIX reset
	in	r5, MX_CMD_RESET | 1\IO_CHAN
	.word	fail, fail, w3, fail
w3:	hlt
	bb	r6, ?X
	ujs	w3
	im	mask0
	hlt	077

fail:	im	mask0
	hlt	040

stack:

; XPCT rz[15] : 0
; XPCT rz[6] : 0
; XPCT alarm : 0
; XPCT r3 : 3
; XPCT ir : 0xec3f
