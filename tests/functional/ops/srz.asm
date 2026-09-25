	; srz should shift right with 0 in (not Y or X), clear Y from LSB, not touch other flags
	lwt	r0, -1
	lw	r1, 0b1010101010101010
	srz	r1
	rpc	r2

	; srz should set Y from LSB, not touch other flags
	lwt	r0, 0
	lw	r3, 0b0101010101010101
	srz	r3
	rpc	r4

	hlt	077

; XPCT r1 : 0b0101010101010101
; XPCT r2 : 0b1111111011111111
; XPCT r3 : 0b0010101010101010
; XPCT r4 : 0b0000000100000000
