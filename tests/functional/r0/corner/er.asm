	lw	r0, 0
	er	r0, 0
	rpc	r1

	lw	r0, 0
	er	r0, 0xffff
	rpc	r2

	lw	r0, 0xffff
	er	r0, 0
	rpc	r3

	lw	r0, 0xffff
	er	r0, 0xffff
	rpc	r4

	hlt	077

; XPCT r1 : 0
; XPCT r2 : 0
; XPCT r3 : 0xffff
; XPCT r4 : 0
