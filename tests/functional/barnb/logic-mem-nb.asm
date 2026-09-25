; om, nm, em, xm should operate on block NB, not on block 0, when called by the OS

	lw	r1, 0b0000000000000001
	ou	r1, 0b0000000000000011
	.word	err, err, ok, err
ok:
	mb	blk

	lw	r1, 0x5a3c
	pw	r1, 100
	pw	r1, 101
	pw	r1, 102
	pw	r1, 103

	lw	r1, 0x0ff0
	om	r1, 100
	nm	r1, 101
	em	r1, 102
	xm	r1, 103

	hlt	077

err:	hlt	040
blk:	.word	1

	.org	100
	.word	0x1111, 0x2222, 0x3333, 0x4444

; XPCT rz[2] : 0
; XPCT [1:100] : 0x5ffc
; XPCT [1:101] : 0x0a30
; XPCT [1:102] : 0x500c
; XPCT [1:103] : 0x55cc
; XPCT [100] : 0x1111
; XPCT [101] : 0x2222
; XPCT [102] : 0x3333
; XPCT [103] : 0x4444
