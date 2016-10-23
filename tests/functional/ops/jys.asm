
	lw r1, 0b1000000000000000
	slz r1
	jys fin
	hlt 040
fin:	hlt 077

; XPCT rz[6] : 0
; XPCT sr : 0

; XPCT ir&0x3f : 0o77
