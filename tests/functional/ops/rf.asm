
	lwt	r1, 10
	lwt	r2, 20
	lwt	r3, 30
	rf	110

	hlt	077

; XPCT rz[6] : 0
; XPCT sr : 0

; XPCT [110] : 10
; XPCT [111] : 20
; XPCT [112] : 30
