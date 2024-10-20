; OPTS -c configs/mega_max.ini

; does PROM allocate only last segment of block 0?

	.include cpu.inc
	.include io.inc
	.include mega.inc

	.const	magic 0x2323
	.const	nb 0
	.const	ab 15\3
	.const	mp 0\14
	.const	seg 2\10

	uj	start

mask:	.word	IMASK_NOMEM

nomem_proc:
	awt	r7, 1
	lip

	.org	OS_START

start:
	lwt	r7, 0

	lw	r1, stack
	rw	r1, STACKP
	lw	r1, nomem_proc
	rw	r1, INTV_NOMEM
	im	mask

	; only 3 fetches should succeed
	tw	r1, 0x0000
	tw	r1, 0x1000
	tw	r1, 0x2000
	tw	r1, 0x3000
	tw	r1, 0x4000
	tw	r1, 0x5000
	tw	r1, 0x6000
	tw	r1, 0x7000
	tw	r1, 0x8000
	tw	r1, 0x9000
	tw	r1, 0xa000
	tw	r1, 0xb000
	tw	r1, 0xc000
	tw	r1, 0xd000
	tw	r1, 0xe000
	tw	r1, 0xf000

	hlt	077
stack:

; XPCT r7 : 13
; XPCT ir : 0xec3f
