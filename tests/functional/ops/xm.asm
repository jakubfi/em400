
	; xm should xor register into memory
	; xm should clear Z on non-zero result, not touch other flags
	lwt	r0, -1
	lw	r1, 0b1010101010101010
	xm	r1, 100
	rpc	r2

	; xm should set Z on zero result, not touch other flags
	lwt	r0, 0
	lw	r3, 0x5a3c
	xm	r3, 101
	rpc	r4

	hlt	077

	.org	100
	.word	0b1111111100000000
	.word	0x5a3c

; XPCT [100] : 0b0101010110101010
; XPCT r1 : 0b1010101010101010
; XPCT r2 : 0x7fff
; XPCT [101] : 0
; XPCT r3 : 0x5a3c
; XPCT r4 : 0x8000
