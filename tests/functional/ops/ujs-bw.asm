
	ujs fin
	.res 4
	hlt 040
fin2:	hlt 077
	.res 5
	hlt 040
fin:	ujs fin2
	.res 5
	hlt 040

; XPCT rz[6] : 0
; XPCT sr : 0

; XPCT ir : 0xec3f
