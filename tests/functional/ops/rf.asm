	; rf should write to block 0 (Q=0), not NB (unconfigured)
	mb	blk

	; rf should store r1-r3 at consecutive addresses, not touch flags or neighbouring words
	lwt	r0, -1
	lw	r1, 0x5a3c
	lw	r2, 0xa5c3
	lw	r3, 0x0f0f
	rf	101
	rpc	r4

	; rf should not set Z on zero, M on negative
	lwt	r0, 0
	lwt	r1, 0
	lw	r2, 0x8000
	rf	201
	rpc	r5

	; rf should store the register used as the argument, not touch registers
	lw	r1, 301
	rf	r1

	hlt	077

blk:	.word	0b001111

	.org	100
	.word	-1, 0x1111, 0x1111, 0x1111, -1
	.org	201
	.word	-1, -1, -1
	.org	301
	.word	-1, -1, -1

; XPCT rz[2] : 0
; XPCT [100] : 0xffff
; XPCT [101] : 0x5a3c
; XPCT [102] : 0xa5c3
; XPCT [103] : 0x0f0f
; XPCT [104] : 0xffff
; XPCT r4 : 0xffff
; XPCT [201] : 0
; XPCT [202] : 0x8000
; XPCT [203] : 0x0f0f
; XPCT r5 : 0
; XPCT [301] : 301
; XPCT [302] : 0x8000
; XPCT [303] : 0x0f0f
; XPCT r1 : 301
; XPCT r2 : 0x8000
; XPCT r3 : 0x0f0f
