
	; nr should do bitwise and, clear Z on non-zero result, not touch other flags
	lwt	r0, -1
	lw	r1, 0b1010101010101010
	nr	r1, 0b1111111100000000
	rpc	r2

	; nr should set Z on zero result, not touch other flags
	lwt	r0, 0
	lw	r3, 0x5a3c
	nr	r3, 0xa5c3
	rpc	r4

	hlt	077

; XPCT r1 : 0b1010101000000000
; XPCT r2 : 0x7fff
; XPCT r3 : 0
; XPCT r4 : 0x8000
