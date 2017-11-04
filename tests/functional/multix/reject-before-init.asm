; OPTS -c configs/multix.cfg

; Check if MULTIX rejects commands before initialization completes

	.include hw.inc
	.include mx.inc

	uj	start

mask:	.word	IMASK_CH0_1
maskz:	.word	IMASK_NONE

mx_proc:
	awt	r4, -1
	jz	good
	hlt	041		; test should end before mx int
good:	hlt	077

	.org	OS_MEM_BEG
start:
	lwt	r4, 0
	lw	r3, stack
	rw	r3, STACKP
	lw	r3, mx_proc
	rw	r3, MX_IV

	mcl

	im	mask

	ou	r5, MX_IO_SETCFG | MX_CHAN
	.word	fail, ok, fail, fail
ok:	lwt	r4, 1	; EN = command rejected -> this is OK
fail:	hlt		; NO, OK, PE -> this is bad
	ujs	fail

stack:

; XPCT rz[15] : 0
; XPCT rz[6] : 0
; XPCT alarm : 0
; XPCT ir : 0xec3f
