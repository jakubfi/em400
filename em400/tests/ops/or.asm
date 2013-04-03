.prog "op/OR"

	lw r1, 0b1010101010101010
	or r1, 0b1111111100000000

	lw r2, 0
	or r2, 0

	hlt 077

.finprog

; XPCT int(rz[6]) : 0
; XPCT int(sr) : 0

; XPCT bin(r1) : 0b1111111110101010
; XPCT int(r2) : 0
; XPCT bin(r0) : 0b1000000000000000
