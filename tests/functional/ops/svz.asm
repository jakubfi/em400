	; svz should shift left with 0 in (not Y or X), clear Y from MSB, not clear V, not touch other flags
	lwt	r0, -1
	lw	r1, 0b0010101010101010
	svz	r1
	rpc	r2

	; svz should set V on 0->1 sign change
	lwt	r0, 0
	lw	r3, 0b0101010101010101
	svz	r3
	rpc	r4

	; svz should set V on 1->0 sign change, set Y from MSB
	lwt	r0, 0
	lw	r5, 0b1010101010101010
	svz	r5
	rpc	r6

	; svz should not set V without sign change
	lwt	r0, 0
	lw	r7, 0b1101010101010101
	svz	r7

	hlt	077

; XPCT r1 : 0b0101010101010100
; XPCT r2 : 0b1111111011111111
; XPCT r3 : 0b1010101010101010
; XPCT r4 : 0b0010000000000000
; XPCT r5 : 0b0101010101010100
; XPCT r6 : 0b0010000100000000
; XPCT r7 : 0b1010101010101010
; XPCT r0 : 0b0000000100000000
