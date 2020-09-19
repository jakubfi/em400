	lw	r0, 0xffff
	ngl	r0
	rpc	r1

	lw	r0, 0
	ngl	r0
	rpc	r2

	hlt	077

; XPCT r1 : 0
; XPCT r2 : 0xffff
