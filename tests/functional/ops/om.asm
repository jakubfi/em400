
	; om should or register into memory
	; om should clear Z on non-zero result, not touch other flags
	lwt	r0, -1
	lw	r1, 0b1010101010101010
	om	r1, 100
	rpc	r2

	; om should set Z on zero result, not touch other flags
	lwt	r0, 0
	lw	r3, 0
	om	r3, 101
	rpc	r4

	hlt	077

	.org	100
	.word	0b1111111100000000
	.word	0

; XPCT [100] : 0b1111111110101010
; XPCT r1 : 0b1010101010101010
; XPCT r2 : 0x7fff
; XPCT [101] : 0
; XPCT r3 : 0
; XPCT r4 : 0x8000
