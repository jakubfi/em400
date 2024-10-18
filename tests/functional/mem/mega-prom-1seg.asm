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
	lw	r1, [0x0000]
	lw	r1, [0x1000]
	lw	r1, [0x2000]
	lw	r1, [0x3000]
	lw	r1, [0x4000]
	lw	r1, [0x5000]
	lw	r1, [0x6000]
	lw	r1, [0x7000]
	lw	r1, [0x8000]
	lw	r1, [0x9000]
	lw	r1, [0xa000]
	lw	r1, [0xb000]
	lw	r1, [0xc000]
	lw	r1, [0xd000]
	lw	r1, [0xe000]
	lw	r1, [0xf000]

	hlt	077
stack:

; XPCT r7 : 13
; XPCT ir : 0xec3f
