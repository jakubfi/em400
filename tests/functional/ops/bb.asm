
	lw r1, 0b1010101010101010
	bb r1, 0b1010101010101010
	hlt 040
	bb r1, 0b1010101010101111
	hlt 077
	hlt 040

; XPCT rz[6] : 0
; XPCT sr : 0

; XPCT ir : 0xec3f
