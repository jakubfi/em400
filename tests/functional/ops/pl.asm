	; pl should store r5-r7 at consecutive addresses, not touch flags or neighbouring words
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

	.org	100
	.word	-1, 0x1111, 0x1111, 0x1111, -1
	.org	201
	.word	-1, -1, -1
	.org	301
	.word	-1, -1, -1

; XPCT [100] : 0xffff
; XPCT [101] : 0x5a3c
; XPCT [102] : 0xa5c3
; XPCT [103] : 0x0f0f
; XPCT [104] : 0xffff
; XPCT r1 : 0xffff
; XPCT [201] : 0
; XPCT [202] : 0x8000
; XPCT [203] : 0x0f0f
; XPCT r2 : 0
; XPCT [301] : 301
; XPCT [302] : 0x8000
; XPCT [303] : 0x0f0f
; XPCT r5 : 301
; XPCT r6 : 0x8000
; XPCT r7 : 0x0f0f
