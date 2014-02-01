
	lw r1, 0b1111111111111111
	ngl r1

	lw r2, 0b0000000000000001
	ngl r2

	hlt 077

; XPCT int(rz[6]) : 0
; XPCT int(sr) : 0

; XPCT bin(r1) : 0b0000000000000000
; XPCT bin(r2) : 0b1111111111111110

