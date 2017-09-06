
	lw	r1, 0b1110101011101010
	bc	r1, 0b1010101010101011
	hlt	040
	bc	r1, 0b1010101010101010
	hlt	077
	hlt	040

; XPCT rz[6] : 0
; XPCT sr : 0

; XPCT ir : 0xec3f
