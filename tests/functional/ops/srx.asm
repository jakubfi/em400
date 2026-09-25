	; srx should shift right with X in, set Y from LSB, not touch other flags
	lw	r0, ~?Y
	lw	r1, 0b0101010101010101
	srx	r1
	rpc	r2

	; srx should shift in X=0 (not Y), clear Y from LSB, not touch other flags
	lw	r0, ?Y
	lw	r3, 0b1010101010101010
	srx	r3
	rpc	r4

	hlt	077

; XPCT r1 : 0b1010101010101010
; XPCT r2 : 0b1111111111111111
; XPCT r3 : 0b0101010101010101
; XPCT r4 : 0b0000000000000000
