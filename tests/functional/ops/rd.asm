
	lwt	r1, 10
	lwt	r2, 20
	rd	120

	hlt	077

; XPCT rz[6] : 0
; XPCT sr : 0

; XPCT [120] : 10
; XPCT [121] : 20
