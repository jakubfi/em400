	; map block 1 page 0 to module 1 frame 0, set NB=1
	lwt	r1, 1
	ou	r1, 0b0000000000000011
	.word	err, err, ok, err
ok:
	mb	blk

	; block 1 is not reloaded with the program, set up words used by the test
	lw	r1, 0x1111
	lwt	r2, -2
clr:	pw	r1, r2+103
	irb	r2, clr

	lwt	r1, 0
	pw	r1, 201
	lw	r1, 0x8000
	pw	r1, 202

	; td should not touch flags or r3
	lwt	r0, -1
	lwt	r3, -1
	td	101
	rpc	r4

	; td should load r1-r2 from consecutive words in block NB, using the address computed before r1 is overwritten
	; td should not set Z on zero, M on negative
	lwt	r0, 0
	lw	r1, 201
	td	r1
	rpc	r5

	hlt	077

err:	hlt	040
blk:	.word	1

	.org	201
	.word	0x2222, 0x2222

; XPCT r4 : 0xffff
; XPCT r3 : 0xffff
; XPCT r1 : 0
; XPCT r2 : 0x8000
; XPCT r5 : 0
