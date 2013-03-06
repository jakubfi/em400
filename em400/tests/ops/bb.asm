.program "op/BB"

	lw r1, 0b1010101010101010
	bb r1, 0b1010101010101010
	hlt 077
	bb r1, 0b1010101010101111
	hlt 077
	hlt 077

.endprog

; XPCT int(rz(6)) : 0
; XPCT int(sr) : 0

; XPCT int(ic) : 8

