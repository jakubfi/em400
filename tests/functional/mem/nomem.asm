; OPTS -c configs/no_user_mem.cfg

	.include cpu.inc

	uj	start

mask:	.word	IMASK_NOMEM | 1
nomem_proc:
	hlt	077

	.org	OS_START
start:
	lw	r3, stack
	rw	r3, STACKP
	lw	r3, nomem_proc
	rw	r3, INTV_NOMEM
	im	mask
	mb	mask

	pw	r1, 10

	hlt	040

stack:

; XPCT rz[6] : 0
; XPCT ir : 0xec3f
