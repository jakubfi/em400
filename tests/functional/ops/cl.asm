
	lw	r1, -1
	cl	r1, 1
	rpc	r2

	lw	r1, 12
	cl	r1, 12
	rpc	r3

	lw	r1, -12
	cl	r1, -12
	rpc	r4

	lw	r1, 1
	cl	r1, -1
	rpc	r5

	hlt	077

; XPCT rz[6] : 0
; XPCT sr : 0

; XPCT r2 : 0b0000001000000000
; XPCT r3 : 0b0000010000000000
; XPCT r4 : 0b0000010000000000
; XPCT r5 : 0b0000100000000000
