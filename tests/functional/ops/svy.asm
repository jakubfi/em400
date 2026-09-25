	; svy should shift left with Y in, clear Y from MSB, not clear V, not touch other flags
	lw	r0, ~?X
	lw	r1, 0b0010101010101010
	svy	r1
	rpc	r2

	; svy should shift in Y=0 (not X), set Y from MSB, set V on 1->0 sign change
	lw	r0, ?X
	lw	r3, 0b1010101010101010
	svy	r3
	rpc	r4

	; svy should set V on 0->1 sign change
	lwt	r0, 0
	lw	r5, 0b0101010101010101
	svy	r5
	rpc	r6

	; svy should not set V without sign change
	lwt	r0, 0
	lw	r7, 0b1101010101010101
	svy	r7

	hlt	077

; XPCT r1 : 0b0101010101010101
; XPCT r2 : 0b1111111001111111
; XPCT r3 : 0b0101010101010100
; XPCT r4 : 0b0010000110000000
; XPCT r5 : 0b1010101010101010
; XPCT r6 : 0b0010000000000000
; XPCT r7 : 0b1010101010101010
; XPCT r0 : 0b0000000100000000
