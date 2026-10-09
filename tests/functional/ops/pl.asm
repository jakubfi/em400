	; map block 1 page 0 to module 1 frame 0, set NB=1
	lwt	r1, 1
	ou	r1, 0b0000000000000011
	.word	err, err, ok, err
ok:
	mb	blk

	; block 1 is not reloaded with the program, clear words checked by the test
	lwt	r1, -1
	lw	r2, -300
clr:	pw	r1, r2+400
	irb	r2, clr

	; pl should store r5-r7 at consecutive addresses in block NB, not touch flags, neighbouring words or block 0
	lwt	r0, -1
	lw	r5, 0x5a3c
	lw	r6, 0xa5c3
	lw	r7, 0x0f0f
	pl	101
	rpc	r1

	; pl should not set Z on zero, M on negative
	lwt	r0, 0
	lwt	r5, 0
	lw	r6, 0x8000
	pl	201
	rpc	r2

	; pl should store the register used as the argument, not touch registers
	lw	r5, 301
	pl	r5

	hlt	077

err:	hlt	040
blk:	.word	1

	.org	101
	.word	0x2222, 0x2222, 0x2222

; XPCT [1:100] : 0xffff
; XPCT [1:101] : 0x5a3c
; XPCT [1:102] : 0xa5c3
; XPCT [1:103] : 0x0f0f
; XPCT [1:104] : 0xffff
; XPCT [101] : 0x2222
; XPCT [102] : 0x2222
; XPCT [103] : 0x2222
; XPCT r1 : 0xffff
; XPCT [1:201] : 0
; XPCT [1:202] : 0x8000
; XPCT [1:203] : 0x0f0f
; XPCT r2 : 0
; XPCT [1:301] : 301
; XPCT [1:302] : 0x8000
; XPCT [1:303] : 0x0f0f
; XPCT r5 : 301
; XPCT r6 : 0x8000
; XPCT r7 : 0x0f0f
