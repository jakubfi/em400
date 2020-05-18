; OPTS -c configs/mega_max.ini

; does MEGA deallocation work?

	.include cpu.inc
	.include io.inc
	.include mega.inc

	.const	magic 0x2323
	.const	nb 1\15
	.const	ab 2\3
	.const	mp 0\14
	.const	seg 2\10

	uj	start

mask:	.word	IMASK_NOMEM | 1

nomem_proc:
	aw	r7, 1
	lip

err:	hlt	040

	.org	OS_START

start:	lwt	r7, 0
	lw	r1, stack
	rw	r1, STACKP
	lw	r1, nomem_proc
	rw	r1, INTV_NOMEM

	lw	r1, ab | nb
	ou	r1, mp | seg | MEGA_ALLOC | MEGA_EPROM_HIDE | MEGA_ALLOC_DONE | MEM_CFG
	.word	err, err, ok, err

ok:	mb	mask
	im	mask
	lw	r1, magic
	pw	r1, ab

	lw	r1, ab | nb
	ou	r1, MEGA_FREE | MEM_CFG
	.word	err, err, ok2, err

ok2:	tw	r1, ab

	lw	r1, ab | nb
	ou	r1, mp | seg | MEGA_ALLOC | MEM_CFG
	.word	err, err, ok3, err

ok3:	tw	r2, ab

	hlt	077
stack:

; XPCT r2 : 0x2323
; XPCT r7 : 1
; XPCT ir : 0xec3f
