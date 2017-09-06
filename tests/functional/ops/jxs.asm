
	lw	r0, 0b0000000010000000
	jxs	fin
	hlt	040
fin:	hlt	077

; XPCT rz[6] : 0
; XPCT sr : 0

; XPCT ir : 0xec3f
