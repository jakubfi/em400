	lw	r0, 0x8000
	slx	r0
	rpc	r1

	lw	r0, ?X
	slx	r0
	rpc	r2

	hlt	077

; XPCT r1 : 0
; XPCT r2 : 0b0000000100000001
