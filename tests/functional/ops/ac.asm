
	lw	r0, ?C
	lw	r1, 10
	ac	r1, 100

	hlt	077

; XPCT rz[6] : 0
; XPCT sr : 0

; XPCT r1 : 111
