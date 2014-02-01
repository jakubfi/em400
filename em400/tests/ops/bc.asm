
	lw r1, 0b1110101011101010
	bc r1, 0b1010101010101011
	hlt 040
	bc r1, 0b1010101010101010
	hlt 077
	hlt 040

; XPCT int(rz[6]) : 0
; XPCT int(sr) : 0

; XPCT oct(ir[10-15]) : 077

