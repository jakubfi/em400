
	lw	r0, ~?L
	jls	fail
	lw	r0, ?L
	jls	fin
fail:	hlt	040
fin:	hlt	077

; XPCT rz[6] : 0
; XPCT sr : 0

; XPCT ir : 0xec3f
