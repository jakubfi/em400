; is should operate on block NB, not on block 0, when called by the OS

	lw	r1, 0b0000000000000001
	ou	r1, 0b0000000000000011
	.word	err, err, ok, err
ok:
	mb	blk

	lw	r1, 0x5a3c
	pw	r1, 100

	lw	r1, 0x0ff0
	is	r1, 100
	hlt	077
	hlt	040

err:	hlt	040
blk:	.word	1

	.org	100
	.word	0xffff

; XPCT rz[2] : 0
; XPCT [1:100] : 0x5ffc
; XPCT [100] : 0xffff
