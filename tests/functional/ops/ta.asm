	; map block 1 page 0 to module 1 frame 0, set NB=1
	lwt	r1, 1
	ou	r1, 0b0000000000000011
	.word	err, err, ok, err
ok:
	mb	blk

	; block 1 is not reloaded with the program, set up words used by the test
	lw	r1, 0x1111
	lwt	r2, -7
clr:	pw	r1, r2+108
	irb	r2, clr

	lwt	r1, 0
	pw	r1, 201
	lw	r1, 0x8000
	pw	r1, 202
	lw	r1, 0x5a3c
	pw	r1, 203
	lw	r1, 0xa5c3
	pw	r1, 204
	lw	r1, 0x0f0f
	pw	r1, 205
	lw	r1, 0xf0f0
	pw	r1, 206
	lw	r1, 0x00ff
	pw	r1, 207

	; ta should not touch flags
	lwt	r0, -1
	ta	101
	rw	r0, 400

	; ta should load r1-r7 from consecutive words in block NB, using the address computed before r1 is overwritten
	; ta should not set Z on zero, M on negative
	lwt	r0, 0
	lw	r1, 201
	ta	r1

	hlt	077

err:	hlt	040
blk:	.word	1

	.org	201
	.word	0x2222, 0x2222, 0x2222, 0x2222, 0x2222, 0x2222, 0x2222
	.org	400
	.word	0x1111

; XPCT [400] : 0xffff
; XPCT r1 : 0
; XPCT r2 : 0x8000
; XPCT r3 : 0x5a3c
; XPCT r4 : 0xa5c3
; XPCT r5 : 0x0f0f
; XPCT r6 : 0xf0f0
; XPCT r7 : 0x00ff
; XPCT r0 : 0
