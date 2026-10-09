	; rb should store the right byte of the register into the left byte (even address), keep the right byte of memory
	; rb should not touch flags or the register
	lwt	r0, -1
	lw	r1, 0x11a5
	rb	r1, 200
	rpc	r2

	; rb should store into the right byte (odd address), keep the left byte of memory, not set Z on zero byte
	lwt	r0, 0
	lw	r3, 0x3300
	rb	r3, 203
	rpc	r4

	hlt	077

	.org	100
	.word	0x5a3c
	.word	0x5a3c

; XPCT [100] : 0xa53c
; XPCT r1 : 0x11a5
; XPCT r2 : 0xffff
; XPCT [101] : 0x5a00
; XPCT r3 : 0x3300
; XPCT r4 : 0
