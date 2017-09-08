; OPTS -c configs/multix.cfg

; Check if MULTIX rejects commands before initialization completes

	.include hw.inc
	.include mx.inc

	uj	start

mask:
	.word	IMASK_CH0_1
mx_proc:
	hlt	041		; test should end before mx int

	.org	OS_MEM_BEG
start:
	lw	r3, stack
	rw	r3, STACKP
	lw	r3, mx_proc
	rw	r3, MX_IV
	im	mask

	ou	r5, MX_IO_SETCFG | MX_CHAN
	.word	fail, ok, fail, fail
ok:	hlt	077	; EN = command rejected -> this is OK
fail:	hlt	040	; NO, OK, PE -> this is bad

stack:

; XPCT rz[15] : 0
; XPCT rz[6] : 0
; XPCT alarm : 0
; XPCT ir : 0xec3f
