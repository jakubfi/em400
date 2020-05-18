; OPTS -c configs/multix.ini

; MULTIX' "check if exists" command should return OK both before and after mx initialization

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

	; "exists" before initialization
	in	r5, MX_CMD_EXISTS | 1\IO_CHAN
	.word	fail, fail, ok, fail
fail:	hlt	040
ok:	im	mask
	hlt		; wait for mx int

	; "exists" after initialization
	in	r5, MX_CMD_EXISTS | 1\IO_CHAN
	.word	fail2, fail2, ok2, fail2
fail2:	hlt	041
ok2:	hlt	077

stack:

; XPCT rz[15] : 0
; XPCT rz[6] : 0
; XPCT alarm : 0
; XPCT r4 : 0b0000001000000000
; XPCT ir : 0xec3f
