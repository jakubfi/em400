	; rd should write to block 0 (Q=0), not NB (unconfigured)
	mb	blk

	; rd should store r1-r2 at consecutive addresses, not touch flags or neighbouring words
	lwt	r0, -1
	lw	r1, 0x5a3c
	lw	r2, 0xa5c3
	rd	101
	rpc	r3

	; rd should not set Z on zero, M on negative
	lwt	r0, 0
	lwt	r1, 0
	lw	r2, 0x8000
	rd	201
	rpc	r4

	; rd should store the register used as the argument, not touch registers
	lw	r1, 301
	rd	r1

	hlt	077

blk:	.word	0b001111

	.org	100
	.word	-1, 0x1111, 0x1111, -1
	.org	201
	.word	-1, -1
	.org	301
	.word	-1, -1

; XPCT rz[2] : 0
; XPCT [100] : 0xffff
; XPCT [101] : 0x5a3c
; XPCT [102] : 0xa5c3
; XPCT [103] : 0xffff
; XPCT r3 : 0xffff
; XPCT [201] : 0
; XPCT [202] : 0x8000
; XPCT r4 : 0
; XPCT [301] : 301
; XPCT [302] : 0x8000
; XPCT r1 : 301
; XPCT r2 : 0x8000
