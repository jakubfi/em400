
	lw r1, 16
	cw r1, 15
	jgs fin
	hlt 040
fin:	hlt 077

; XPCT rz[6] : 0
; XPCT sr : 0

; XPCT ir : 0xec3f
