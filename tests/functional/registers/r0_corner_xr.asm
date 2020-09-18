	lw	r0, 0
	xr	r0, 0
	rpc	r1

	lw	r0, 0
	xr	r0, 0xffff
	rpc	r2

	lw	r0, 0xffff
	xr	r0, 0
	rpc	r3

	lw	r0, 0xffff
	xr	r0, 0xffff
	rpc	r4

	hlt	077

; XPCT r1 : 0
; XPCT r2 : 0xffff
; XPCT r3 : 0xffff
; XPCT r4 : 0
