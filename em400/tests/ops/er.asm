
	lw r1, 0b1010101010101010
	er r1, 0b1111111100000000

	lw r2, 0
	er r2, 0

	hlt 077

; XPCT int(rz[6]) : 0
; XPCT int(sr) : 0

; XPCT bin(r1) : 0b0000000010101010
; XPCT int(r2) : 0
; XPCT bin(r0) : 0b1000000000000000
