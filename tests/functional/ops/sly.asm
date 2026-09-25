	; sly should shift left with Y in, clear Y from MSB, not touch other flags
	lw	r0, ~?X
	lw	r1, 0b0101010101010101
	sly	r1
	rpc	r2

	; sly should shift in Y=0 (not X), set Y from MSB, not set V on sign change, not touch other flags
	lw	r0, ?X
	lw	r3, 0b1010101010101010
	sly	r3
	rpc	r4

	hlt	077

; XPCT r1 : 0b1010101010101011
; XPCT r2 : 0b1111111001111111
; XPCT r3 : 0b0101010101010100
; XPCT r4 : 0b0000000110000000
