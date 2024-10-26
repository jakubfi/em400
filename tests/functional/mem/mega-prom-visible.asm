; OPTS -c configs/mega_max.ini

; is write to PROM shown by default?

	.include cpu.inc
	.include io.inc
	.include mega.inc

	uj	start

mask:	.word   IMASK_NOMEM | IMASK_PARITY
nomem_proc:
	hlt	050

	.org	OS_START
start:
	lw	r1, stack
	rw	r1, STACKP
	lw	r1, nomem_proc
	rw	r1, INTV_NOMEM
	rw	r1, INTV_PARITY
	im	mask

test_ro:
	; access within PROM page should not fail
	lw	r1, [0xffff]
	hlt	077

stack:

; XPCT ir : 0xec3f
