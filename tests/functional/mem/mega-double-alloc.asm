; OPTS -c configs/mega_max.cfg

; does MEGA allow to allocate the same physical segment for two logical segments?

	.include cpu.inc
	.include io.inc
	.include mega.inc

	.const	magic 0x2323
	.const	nb 0
	.const	ab1 2\3
	.const	ab2 3\3
	.const	mp 0\14
	.const	seg 2\10
	.const	addr 100

	uj	start

mask:	.word	IMASK_NOMEM

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

	lw	r1, ab1 | nb
	ou	r1, seg | mp | MEM_CFG | MEGA_EPROM_HIDE | MEGA_ALLOC_DONE | MEGA_ALLOC
	.word	err, err, ok, err

ok:	lw	r1, ab2 | nb
	ou	r1, seg | mp | MEM_CFG | MEGA_ALLOC
	.word	err, err, ok2, err

ok2:	im	mask
	lw	r1, magic
	rw	r1, ab1 + addr
	lw	r2, [ab1+addr]
	lw	r3, [ab2+addr]

	hlt	077
stack:

; XPCT r2 : 0x2323
; XPCT r3 : 0x2323
; XPCT r7 : 0
; XPCT ir : 0xec3f
