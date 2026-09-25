	; slx should shift left with X in, set Y from MSB, not touch other flags
	lw	r0, ~?Y
	lw	r1, 0b1101010101010101
	slx	r1
	rpc	r2

	; slx should shift in X=0 (not Y), clear Y from MSB, not set V on sign change, not touch other flags
	lw	r0, ?Y
	lw	r3, 0b0101010101010101
	slx	r3
	rpc	r4

	hlt	077

; XPCT r1 : 0b1010101010101011
; XPCT r2 : 0b1111111111111111
; XPCT r3 : 0b1010101010101010
; XPCT r4 : 0b0000000000000000
