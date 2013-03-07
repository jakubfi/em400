.program "op/LB"

; PRE [10] = 0b0101010111001100

	lw r1, 0b1010101010101010
	lb r1, 21
	lw r2, 0b0101010101010101
	lb r2, 20
	hlt 077

.endprog

; XPCT int(rz(6)) : 0
; XPCT int(sr) : 0

; XPCT bin(r1) : 0b1010101011001100
; XPCT bin(r2) : 0b0101010101010101

