; OPTS -c configs/mega_max.ini

; does PROM hide/show work?
; are PROM contents the same after hide/show?

	.include cpu.inc
	.include io.inc
	.include mega.inc

	.const	SEGMENT 0\MEM_SEGMENT
	.const	PAGE 2\MEM_PAGE
	.const	MODULE 0\MEM_MODULE
	.const	FRAME 2\MEM_FRAME

	uj	start

mask:	.word	IMASK_NOMEM

nomem_proc:
	lw	r7, 1
	lip
nomem_proc2:
	hlt	040

	.org	OS_START

start:	lwt	r7, 0
	; initialize interrupts
	lw	r1, stack
	rw	r1, STACKP
	lw	r1, nomem_proc2
	rw	r1, INTV_NOMEM
	im	mask

	; initial read should not fail, PROM is shown by default
	tw	r6, 0xffff
	cwt	r6, 0		; read value is the sysnum, should be != 0
	je	fail_sysnum

	; change interrupt vector
	lw	r1, nomem_proc
	rw	r1, INTV_NOMEM
	; hide PROM
	lw	r1, PAGE | SEGMENT
	ou	r1, MODULE | FRAME | MEGA_ALLOC | MEGA_EPROM_HIDE | MEGA_ALLOC_DONE | MEM_CFG
	.word	.err1, .err1, .ok1, .err1
.err1:	hlt	041
.ok1:
	; this should fail leaving "1" in r7
	tw	r1, 0xffff
	cw	r7, 1
	jes	.read_prom
	hlt	042
.read_prom:
	; change interrupt vector again
	lw	r1, nomem_proc2
	rw	r1, INTV_NOMEM

	; show PROM again
	lw	r1, PAGE | SEGMENT
	ou	r1, MODULE | FRAME | MEGA_ALLOC | MEGA_EPROM_SHOW | MEGA_ALLOC_DONE | MEM_CFG
	.word	.err2, .err2, .ok2, .err2
.err2:	hlt	043
.ok2:
	; this sould read fine
	tw	r1, 0xffff
	cw	r1, r6		; should be the same as the first read
	jes	pass
	hlt	044
pass:
	hlt	077

fail_sysnum:
	hlt	045

stack:

; XPCT ir : 0xec3f
