	; sry should shift right with Y in, clear Y from LSB, not touch other flags
	lw	r0, ~?X
	lw	r1, 0b1010101010101010
	sry	r1
	rpc	r2

	; sry should shift in Y=0 (not X), set Y from LSB, not touch other flags
	lw	r0, ?X
	lw	r3, 0b0101010101010101
	sry	r3
	rpc	r4

	hlt	077

; XPCT r1 : 0b1101010101010101
; XPCT r2 : 0b1111111001111111
; XPCT r3 : 0b0010101010101010
; XPCT r4 : 0b0000000110000000
