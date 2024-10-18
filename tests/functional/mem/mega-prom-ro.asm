; OPTS -c configs/mega_max.ini

; is write to PROM ineffective?

	.include cpu.inc
	.include io.inc
	.include mega.inc

	.const	magic 0x2323
	.const	nb 0
	.const	ab 15\3
	.const	mp 0\14
	.const	seg 2\10

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
	; write onto MEGA PROM should be discarded
	lw	r1, [0xf000]
	lw	r2, r1+0xa5a5
	rw	r2, 0xf000
	cw	r2, [0xf000]
	jes	fail
	hlt	077
fail:
	hlt	040

stack:

; XPCT ir : 0xec3f
