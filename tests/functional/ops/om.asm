
	.include cpu.inc

	lw	r3, stack
	rw	r3, STACKP
	lw	r3, nomem_proc
	rw	r3, INTV_NOMEM

	lw	r1, 0b0000000000000001
	ou	r1, 0b0000000000000011
	.word	err, err, ok, err
ok:
	mb	blk
	im	blk

	lwt	r1, 0
	pw	r1, 10
	pw	r1, 11

	lw	r1, 0b1111111100000000
	om	r1, 10
	lw	r1, 0b0000001101010101
	om	r1, 10

	lw	r2, 0
	om	r2, 11

	hlt	077

err:	hlt	040
blk:	.word	IMASK_NOMEM | 1

nomem_proc:
	hlt	040
stack:

; XPCT rz[6] : 0
; XPCT sr : 0b0100000000000001

; XPCT [1:10] : 0b1111111101010101
; XPCT [1:11] : 0
; XPCT r0 : 0b1000000000000000
