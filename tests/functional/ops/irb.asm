
	lwt	r1, -2
	irb	r1, fin
	hlt	040
fin:	hlt	077

; XPCT rz[6] : 0
; XPCT sr : 0

; XPCT ir : 0xec3f
