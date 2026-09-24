
	lw	r1, 0xffff
	; memory read will fail, but instruction will execute anyway
	; with '0' as the load argument
	lw	r1, [0x8000]
	hlt	077

; XPCT rz[6] : 0
; XPCT r1 : 0
