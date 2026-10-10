	lw	r0, 100
	ri	r0, 0x1111

	hlt	077

; XPCT [100] : 0x1111
; XPCT r0 : 101
