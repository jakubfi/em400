; illegal instruction resets MOD

	lw	r1, 0xffff
	lw	r1, [0x8000] ; memory read will fail, but instruction will execute anyway with '0' as the load argument
	hlt	077

; XPCT rz[6] : 0
; XPCT r1 : 0
