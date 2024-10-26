; OPTS -c configs/mega_max.ini

; does MEGA allow to allocate the same frame to two different pages?

	.include cpu.inc
	.include io.inc
	.include mega.inc

	.const	MAGIC 0xa5a5
	.const	SEGMENT 0\MEM_SEGMENT
	.const	PAGE1 2\MEM_PAGE
	.const	PAGE2 3\MEM_PAGE
	.const	MODULE 0\MEM_MODULE
	.const	FRAME 2\MEM_FRAME
	.const	ADDR 100

	uj	start

mask:	.word	IMASK_NOMEM

nomem_proc:
	aw	r7, 1
	lip
err:	hlt	040

	.org	OS_START

start:	lwt	r7, 0
	; initialize interrupts
	lw	r1, stack
	rw	r1, STACKP
	lw	r1, nomem_proc
	rw	r1, INTV_NOMEM
	im	mask

	; allocate frame to page1
	lw	r1, PAGE1 | SEGMENT
	ou	r1, FRAME | MODULE | MEM_CFG | MEGA_ALLOC_DONE | MEGA_ALLOC
	.word	err, err, ok, err

ok:	; allocate the same frame to page2
	lw	r1, PAGE2 | SEGMENT
	ou	r1, FRAME | MODULE | MEM_CFG | MEGA_ALLOC
	.word	err, err, ok2, err

ok2:	; store MAGIC, read from both locations - should be the same
	lw	r1, MAGIC
	pw	r1, PAGE1 + ADDR
	tw	r2, PAGE1 + ADDR
	tw	r3, PAGE2 + ADDR

	hlt	077
stack:

; XPCT r2 : 0xa5a5
; XPCT r3 : 0xa5a5
; XPCT r7 : 0
; XPCT ir : 0xec3f
