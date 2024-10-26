; Elwro frame address is 3-bit

	.include cpu.inc
	.include io.inc

	.const	MAGIC 0x5a5a
	.const	SEGMENT 1\MEM_SEGMENT
	.const	PAGE1 2\MEM_PAGE
	.const	PAGE2 3\3
	.const	MODULE 0\MEM_MODULE
	.const	FRAME3BIT 0b0010\MEM_FRAME
	.const	FRAME4BIT 0b1010\MEM_FRAME ; both should be the same for Elwro
	.const	addr 100

	uj	start

mask:	.word	IMASK_NOMEM | 1

nomem_proc:
	aw	r7, 1
	lip
err:	hlt	040

	.org	OS_START

start:
	; cear segfault counter
	lwt	r7, 0
	; initialize interrupt system
	lw	r1, stack
	rw	r1, STACKP
	lw	r1, nomem_proc
	rw	r1, INTV_NOMEM
	im	mask

	; configure module:frame as segment:page1
	lw	r1, SEGMENT | PAGE1
	ou	r1, MODULE | FRAME3BIT | MEM_CFG
	.word	err, err, ok, err

ok:
	; store MAGIC in the newly allocated page
	mb	mask
	lw	r1, MAGIC
	pw	r1, PAGE1 + addr

	; configure module:frame as segment:page2
	; but this time frame number is 4-bit, which should be ignored
	lw	r1, SEGMENT | PAGE2
	ou	r1, MODULE | FRAME4BIT | MEM_CFG
	.word	err, err, ok2, err

ok2:
	; read from the "old" page should fail
	tw	r1, PAGE1 + addr
	; read from the "new" page should return MAGIC
	tw	r2, PAGE2 + addr

	hlt	077

stack:

; XPCT r2 : 0x5a5a
; XPCT r7 : 1
; XPCT ir : 0xec3f
