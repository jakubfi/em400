	; slz should shift left with 0 in (not Y or X), clear Y from MSB, not touch other flags
	lwt	r0, -1
	lw	r1, 0b0101010101010101
	slz	r1
	rpc	r2

	; slz should set Y from MSB, not set V on sign change, not touch other flags
	lwt	r0, 0
	lw	r3, 0b1010101010101010
	slz	r3
	rpc	r4

	hlt	077

; XPCT r1 : 0b1010101010101010
; XPCT r2 : 0b1111111011111111
; XPCT r3 : 0b0101010101010100
; XPCT r4 : 0b0000000100000000
