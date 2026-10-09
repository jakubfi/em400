; lb, rb, cb should operate on block NB, not on block 0, when called by the OS

	lw	r1, 0b0000000000000001
	ou	r1, 0b0000000000000011
	.word	err, err, ok, err
ok:
	mb	blk

	lw	r1, 0x5a3c
	pw	r1, 100
	pw	r1, 101

	lwt	r2, 0
	lb	r2, 200

	lw	r3, 0x00a5
	rb	r3, 202

	lwt	r0, 0
	lw	r4, 0x005a
	cb	r4, 200
	rpc	r5

	hlt	077

err:	hlt	040
blk:	.word	1

	.org	100
	.word	0x1111, 0x2222

; XPCT rz[2] : 0
; XPCT r2 : 0x005a
; XPCT [1:101] : 0xa53c
; XPCT [101] : 0x2222
; XPCT r5 : 0x0400
