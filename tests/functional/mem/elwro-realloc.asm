; does Elwro prohibit allocating the same physical segment for two logical segments?
; is data in physical segment preserved between allocations?

	.include cpu.inc
	.include io.inc

	.const	magic 0x2323
	.const	nb 1\15
	.const	ab1 2\3
	.const	ab2 3\3
	.const	mp 0\14
	.const	seg 2\10
	.const	addr 100

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

	lw	r1, ab1 | nb
	ou	r1, mp | seg | MEM_CFG
	.word	err, err, ok, err

ok:	mb	mask
	im	mask
	lw	r1, magic
	pw	r1, ab1 + addr

	lw	r1, ab2 | nb
	ou	r1, mp | seg | MEM_CFG
	.word	err, err, ok2, err

ok2:	tw	r1, ab1+addr
	tw	r2, ab2+addr

	hlt	077

stack:

; XPCT r2 : 0x2323
; XPCT r7 : 1
; XPCT ir : 0xec3f
