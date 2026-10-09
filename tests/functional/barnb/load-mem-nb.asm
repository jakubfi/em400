; tw, td, tf, tl, ta should read from block NB, not from block 0, when called by the OS

	lwt	r1, 1
	ou	r1, 0b0000000000000011
	.word	err, err, ok, err
ok:
	mb	blk

	lw	r1, 0x0a01
	lw	r2, 0x0a02
	lw	r3, 0x0a03
	lw	r4, 0x0a04
	lw	r5, 0x0a05
	lw	r6, 0x0a06
	lw	r7, 0x0a07
	pa	140
	lw	r5, 0x0c05
	lw	r6, 0x0c06
	lw	r7, 0x0c07
	pl	130
	lw	r1, 0x0f01
	lw	r2, 0x0f02
	lw	r3, 0x0f03
	pf	120
	lw	r1, 0x0d01
	lw	r2, 0x0d02
	pd	110
	lw	r1, 0x5a3c
	pw	r1, 100

	lwt	r1, 0
	lwt	r2, 0
	lwt	r3, 0
	lwt	r4, 0
	lwt	r5, 0
	lwt	r6, 0
	lwt	r7, 0

	; each load leaves at least one register that later loads don't overwrite
	ta	140
	tl	130
	tf	120
	td	110
	tw	r0, 100

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
; XPCT r4 : 0x0a04
; XPCT r5 : 0x0c05
; XPCT r6 : 0x0c06
; XPCT r7 : 0x0c07
; XPCT r3 : 0x0f03
; XPCT r1 : 0x0d01
; XPCT r2 : 0x0d02
; XPCT r0 : 0x5a3c
