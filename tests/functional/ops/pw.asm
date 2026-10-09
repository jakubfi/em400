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

	; pw should store the register in block NB, not touch flags, the register, neighbouring words or block 0
	lwt	r0, -1
	lw	r1, 0x5a3c
	pw	r1, 101
	rpc	r2

	; pw should not set Z on zero
	lwt	r0, 0
	lwt	r3, 0
	pw	r3, 201
	rpc	r4

	; pw should store the register used as the argument
	lw	r5, 301
	pw	r5, r5

	; pw r0 should store flags
	lw	r0, 0xa5c3
	pw	r0, 302

	hlt	077

err:	hlt	040
blk:	.word	1

	.org	101
	.word	0x2222

; XPCT [1:100] : 0xffff
; XPCT [1:101] : 0x5a3c
; XPCT [1:102] : 0xffff
; XPCT [101] : 0x2222
; XPCT r1 : 0x5a3c
; XPCT r2 : 0xffff
; XPCT [1:201] : 0
; XPCT r3 : 0
; XPCT r4 : 0
; XPCT [1:301] : 301
; XPCT r5 : 301
; XPCT [1:302] : 0xa5c3
