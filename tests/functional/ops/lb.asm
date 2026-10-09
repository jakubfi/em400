	; lb should load the left byte (even address) into the right byte of the register, keep the left byte of the register
	; lb should not touch flags
	lwt	r0, -1
	lw	r1, 0x1122
	lb	r1, 200
	rpc	r2

	; lb should load the right byte (odd address), not set Z on zero byte
	lwt	r0, 0
	lw	r3, 0x3344
	lb	r3, 203
	rpc	r4

	hlt	077

	.org	100
	.word	0xa5c3
	.word	0x5a00

; XPCT r1 : 0x11a5
; XPCT r2 : 0xffff
; XPCT r3 : 0x3300
; XPCT r4 : 0
