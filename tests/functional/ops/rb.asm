
	.include hw.inc

	lw	r3, stack
	rw	r3, STACKP
	lw	r3, nomem_proc
	rw	r3, IV_NOMEM

	lw	r1, 0b0000000000000001
	ou	r1, 0b0000000000000011
	.word	err, err, ok, err
ok:
	mb	blk
	im	blk

	lw	r1, 0b0001100010101010
	rb	r1, 20

	lw	r2, 0b0001100011001100
	rb	r2, 21

	hlt	077

err:	hlt	040
blk:	.word	IMASK_NOMEM | 1

nomem_proc:
	hlt	040
stack:

; XPCT rz[6] : 0
; XPCT sr : 0b0100000000000001

; XPCT [1:10] : 0b1010101011001100
; XPCT ir : 0xec3f
