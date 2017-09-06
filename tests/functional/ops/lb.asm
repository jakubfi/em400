
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

	lw	r1, 0b0101010111001100
	pw	r1, 10

	lw	r1, 0b1010101010101010
	lb	r1, 21
	lw	r2, 0b0101010101010101
	lb	r2, 20
	hlt	077

err:	hlt	040
blk:	.word	IMASK_NOMEM + 1

nomem_proc:
	hlt	040
stack:

; XPCT rz[6] : 0
; XPCT sr : 0b0100000000000001

; XPCT r1 : 0b1010101011001100
; XPCT r2 : 0b0101010101010101
; XPCT ir : 0xec3f
