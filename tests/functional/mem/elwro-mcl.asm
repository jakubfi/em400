; memory configuration is reset on MCL

	.cpu mera400

	.include cpu.inc
	.include io.inc

	.equ SEGMENT 1
	.equ PAGE 0
	.equ MODULE 1
	.equ FRAME 3

	uj	start

ba:	.word	0b0100000000000000 | SEGMENT\15

nomem_proc:
	hlt	045
nomem_proc2:
	hlt	077

	.org	OS_START

start:	lw	r1, stack
	rw	r1, STACKP
	lwt	r1, nomem_proc
	rw	r1, INTV_NOMEM

	lw	r1, PAGE\MEM_PAGE | SEGMENT\MEM_SEGMENT
	lw	r2, FRAME\MEM_FRAME | MODULE\MEM_MODULE | MEM_CFG
	ou	r1, r2
	.word	err, err, ok, err
err:	hlt	044

ok:	mb	ba
	im	ba
	; this should succeed, segment is configured
	tw	r1, 1

	mcl
	lwt	r1, nomem_proc2
	rw	r1, INTV_NOMEM
	mb	ba
	im	ba

	; this should generate interrupt
	tw	r1, 1

	hlt	046

stack:

; XPCT ir : 0xec3f
