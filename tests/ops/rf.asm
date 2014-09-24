
	lwt	r1, 10
	lwt	r2, 20
	lwt	r3, 30
	rf	110

	hlt	077

; XPCT int(rz[6]) : 0
; XPCT int(sr) : 0

; XPCT int([110]) : 10
; XPCT int([111]) : 20
; XPCT int([112]) : 30
