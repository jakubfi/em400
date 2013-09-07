.prog "op/BB"

	lw r1, 0b1010101010101010
	bb r1, 0b1010101010101010
	hlt 040
	bb r1, 0b1010101010101111
	hlt 077
	hlt 040

.finprog

; XPCT int(rz[6]) : 0
; XPCT int(sr) : 0

; XPCT oct(ir[10-15]) : 077

