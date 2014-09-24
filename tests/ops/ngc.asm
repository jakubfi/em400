
	lw	r0, ?C
	lw	r1, 0b1111111111111111
	ngc	r1

	lw	r2, 0b0000000000000001
	ngc	r2

	hlt	077

; XPCT int(rz[6]) : 0
; XPCT int(sr) : 0

; XPCT bin(r1) : 0b0000000000000001
; XPCT bin(r2) : 0b1111111111111110
