	; cb should set E when the right byte of the register equals the left byte (even address)
	; cb should ignore the left byte of the register, not touch flags other than L, E, G
	lwt	r0, -1
	lw	r1, 0xff80
	cb	r1, 200
	rpc	r2

	; cb should set L when the register byte is lower, compare bytes as unsigned
	lwt	r0, 0
	lw	r3, 0x007f
	cb	r3, 200
	rpc	r4

	; cb should set G when the register byte is greater than the right byte (odd address)
	lw	r5, 0x0013
	cb	r5, 201
	rpc	r6

	hlt	077

	.org	100
	.word	0x8012

; XPCT r2 : 0xf5ff
; XPCT r4 : 0x0800
; XPCT r6 : 0x0200
