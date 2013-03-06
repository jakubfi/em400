.program "op/BC"

	lw r1, 0b1110101011101010
	bc r1, 0b1010101010101011
	hlt 077
	bc r1, 0b1010101010101010
	hlt 077
	hlt 077

.endprog

; XPCT int(rz(6)) : 0
; XPCT int(sr) : 0

; XPCT int(ic) : 8

