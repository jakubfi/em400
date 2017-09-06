; pre-modification fails on 4th time

	md	1
	md	1
	md	1
	md	1
	lw	r1, 1

	hlt	077

; XPCT sr : 0

; XPCT rz[6] : 1
; XPCT r1 : 1
