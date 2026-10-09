; pw, pd, pf, pl, pa should write to block NB, not to block 0, when called by the OS

	lwt	r1, 1
	ou	r1, 0b0000000000000011
	.word	err, err, ok, err
ok:
	mb	blk

	lw	r1, 0x1001
	lw	r2, 0x1002
	lw	r3, 0x1003
	lw	r4, 0x1004
	lw	r5, 0x1005
	lw	r6, 0x1006
	lw	r7, 0x1007

	pw	r1, 100
	pd	110
	pf	120
	pl	130
	pa	140

	hlt	077

err:	hlt	040
blk:	.word	1

	.org	100
	.word	0x1111
	.org	110
	.word	0x1111, 0x1111
	.org	120
	.word	0x1111, 0x1111, 0x1111
	.org	130
	.word	0x1111, 0x1111, 0x1111
	.org	140
	.word	0x1111, 0x1111, 0x1111, 0x1111, 0x1111, 0x1111, 0x1111

; XPCT rz[2] : 0
; XPCT [1:100] : 0x1001
; XPCT [100] : 0x1111
; XPCT [1:110] : 0x1001
; XPCT [1:111] : 0x1002
; XPCT [110] : 0x1111
; XPCT [1:120] : 0x1001
; XPCT [1:122] : 0x1003
; XPCT [120] : 0x1111
; XPCT [1:130] : 0x1005
; XPCT [1:132] : 0x1007
; XPCT [130] : 0x1111
; XPCT [1:140] : 0x1001
; XPCT [1:146] : 0x1007
; XPCT [140] : 0x1111
