; sp should load the context from block NB, not from block 0, when called by the OS

	lw	r1, 0b0000000000000001
	ou	r1, 0b0000000000000011
	.word	err, err, ok, err
ok:
	mb	blk

	lw	r1, good
	pw	r1, 100
	lw	r1, 0x5a3c
	pw	r1, 101
	lwt	r1, 0
	pw	r1, 102

	sp	100
	hlt	040

good:	hlt	077
err:	hlt	040
blk:	.word	1

	.org	100
	.word	err, 0x1111, 1

; XPCT rz[2] : 0
; XPCT r0 : 0x5a3c
; XPCT sr : 0
