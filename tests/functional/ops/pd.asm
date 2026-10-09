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

	; pd should store r1-r2 at consecutive addresses in block NB, not touch flags, neighbouring words or block 0
	lwt	r0, -1
	lw	r1, 0x5a3c
	lw	r2, 0xa5c3
	pd	101
	rpc	r3

	; pd should not set Z on zero, M on negative
	lwt	r0, 0
	lwt	r1, 0
	lw	r2, 0x8000
	pd	201
	rpc	r4

	; pd should store the register used as the argument, not touch registers
	lw	r1, 301
	pd	r1

	hlt	077

err:	hlt	040
blk:	.word	1

	.org	101
	.word	0x2222, 0x2222

; XPCT [1:100] : 0xffff
; XPCT [1:101] : 0x5a3c
; XPCT [1:102] : 0xa5c3
; XPCT [1:103] : 0xffff
; XPCT [101] : 0x2222
; XPCT [102] : 0x2222
; XPCT r3 : 0xffff
; XPCT [1:201] : 0
; XPCT [1:202] : 0x8000
; XPCT r4 : 0
; XPCT [1:301] : 301
; XPCT [1:302] : 0x8000
; XPCT r1 : 301
; XPCT r2 : 0x8000
