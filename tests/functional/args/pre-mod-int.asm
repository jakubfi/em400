; CONFIG configs/minimal-nomem-nostop.ini

; interrupt raised during MD is served only after the pre-modified instruction

	.include cpu.inc

	uj	start

	.org	OS_START
start:
	lw	r1, nomem_proc
	rw	r1, INTV_NOMEM
	lw	r1, stack
	rw	r1, STACKP
	im	mask
	md	[0x8000]	; reads 0 and raises "no memory"
	md	2
	lwt	r2, 3
ret:	hlt	077

nomem_proc:
	lwt	r4, 1
	lw	r3, [stack]
	cw	r3, ret
	jes	1
	hlt	040
	lip

mask:	.word	IMASK_NOMEM
stack:

; XPCT ir : 0xec3f
; XPCT rz[6] : 0

; XPCT r2 : 5
; XPCT r4 : 1
