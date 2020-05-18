; OPTS -c configs/multix.ini

; Test "REQUEUE INTERRUPT" command
; The only thing we can test here is if MULTIX acknowledges the command.
; Everything else happens under the blanket and cannot be verified from CPU perspective

	.include cpu.inc
	.include io.inc
	.include multix.inc

	uj	start

mask:
	.word	IMASK_CH0_1
mx_proc:
	lw	r4, [STACKP]
	lw	r4, [r4-1]
	cw	r4, MX_IWYZE	; is it IWYZE?
	bb	r0, ?E
	hlt	042		; not iwyze
	lip

	.org	OS_START
start:
	lw	r3, stack
	rw	r3, STACKP
	lw	r3, mx_proc
	rw	r3, INTV_CH1

	mcl

	im	mask
	hlt		; wait for mx int

repeat:	in	r5, MX_CMD_REQUEUE | 0\10 | 1\IO_CHAN
	.word	fail, repeat, ok, fail
fail:	hlt	041
ok:	hlt	077

stack:

; XPCT rz[15] : 0
; XPCT rz[6] : 0
; XPCT alarm : 0
; XPCT ir : 0xec3f
