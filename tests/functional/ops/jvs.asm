
	lw r1, 0b0100000000000000
	svz r1
	jvs fin
	hlt 040
fin:	hlt 077

; XPCT rz[6] : 0
; XPCT sr : 0

; XPCT ir : 0xec3f
; XPCT r0 : 0b0000000000000000
