	; map block 1 page 0 to module 1 frame 0, set NB=1
	lwt	r1, 1
	ou	r1, 0b0000000000000011
	.word	err, err, ok, err
ok:
	mb	blk

	; block 1 is not reloaded with the program, set up words used by the test
	lw	r1, 0x1111
	lwt	r2, -3
clr:	pw	r1, r2+104
	irb	r2, clr

	lwt	r1, 0
	pw	r1, 201
	lw	r1, 0x8000
	pw	r1, 202
	lw	r1, 0x5a3c
	pw	r1, 203

	; tf should not touch flags or r4
	lwt	r0, -1
	lwt	r4, -1
	tf	101
	rpc	r5

	; tf should load r1-r3 from consecutive words in block NB, using the address computed before r1 is overwritten
	; tf should not set Z on zero, M on negative
	lwt	r0, 0
	lw	r1, 201
	tf	r1
	rpc	r6

	hlt	077

err:	hlt	040
blk:	.word	1

	.org	201
	.word	0x2222, 0x2222, 0x2222

; XPCT r5 : 0xffff
; XPCT r4 : 0xffff
; XPCT r1 : 0
; XPCT r2 : 0x8000
; XPCT r3 : 0x5a3c
; XPCT r6 : 0
