	lw	r0, 1
	sry	r0
	rpc	r1

	lw	r0, ?G
	sry	r0
	rpc	r2

	hlt	077

; XPCT r1 : 0
; XPCT r2 : 0b0000_000_10_0000000
