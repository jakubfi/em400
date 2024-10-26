; OPTS -c configs/mega_max.ini

; does MEGA deallocation work?
; does reallocating the same segment preserves the data?
; does reallocation without MEGA_ALLOC_DONE work?

	.include cpu.inc
	.include io.inc
	.include mega.inc

	.const	MAGIC 0xa5a5
	.const	SEGMENT 1\MEM_SEGMENT
	.const	PAGE 2\MEM_PAGE
	.const	MODULE 0\MEM_MODULE
	.const	FRAME 2\MEM_FRAME

	uj	start

mask:	.word	IMASK_NOMEM | 1

nomem_proc:
	aw	r7, 1
	lip

err:	hlt	040

	.org	OS_START

start:
	; initialize interrupts
	lwt	r7, 0
	lw	r1, stack
	rw	r1, STACKP
	lw	r1, nomem_proc
	rw	r1, INTV_NOMEM
	im	mask

	; allocate one page, finish mega allocation
	lw	r1, PAGE | SEGMENT
	ou	r1, MODULE | FRAME | MEGA_ALLOC | MEGA_ALLOC_DONE | MEM_CFG
	.word	err, err, ok, err

ok:	; store MAGIC in that page
	mb	mask
	lw	r1, MAGIC
	pw	r1, PAGE

	; deallocate the page
	lw	r1, PAGE | SEGMENT
	ou	r1, MEGA_FREE | MEM_CFG
	.word	err, err, ok2, err

ok2:	; try reading the page (this should fail with "nomem" interrupt)
	tw	r1, PAGE

	; allocate the page again (no MEGA_ALLOC_DONE this time)
	lw	r1, PAGE | SEGMENT
	ou	r1, MODULE | FRAME | MEGA_ALLOC | MEM_CFG
	.word	err, err, ok3, err

ok3:	; read should succeed this time, MAGIC should be read
	tw	r2, PAGE
	cw	r2, MAGIC
	jes	fin
	hlt	041
fin:
	hlt	077
stack:

; XPCT r2 : 0xa5a5
; XPCT r7 : 1
; XPCT ir : 0xec3f
