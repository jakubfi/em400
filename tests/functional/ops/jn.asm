
	lw r1, 15
	cw r1, 15
	jn fin
	hlt 077
fin:	hlt 040

; XPCT rz[6] : 0
; XPCT sr : 0

; XPCT ir&0x3f : 0o77
