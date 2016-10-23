
	lwt r1, 0
	lwt r2, 63
	lwt r3, -63

	hlt 077

; XPCT rz[6] : 0
; XPCT sr : 0

; XPCT r1 : 0
; XPCT r2 : 63
; XPCT r3 : -63
