
	lwt	r1, 10
	lwt	r2, 20
	rd	120

	hlt	077

; XPCT int(rz[6]) : 0
; XPCT int(sr) : 0

; XPCT int([120]) : 10
; XPCT int([121]) : 20
