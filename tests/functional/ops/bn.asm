
	lw r1, 0b1010101010101010
	bn r1, 0b0101010101010101
	hlt 040
	bn r1, 0b0101010101010111
	hlt 077
	hlt 040

; XPCT rz[6] : 0
; XPCT sr : 0

; XPCT ir : 0xec3f
