
	lw	r0, 0b1101011010101101

	brc	0b10101111
	hlt	040
	brc	0b10101101
	hlt	077
	hlt	040

; XPCT rz[6] : 0
; XPCT sr : 0

; XPCT ir&0x3f : 0o77
