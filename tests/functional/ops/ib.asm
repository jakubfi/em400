
	lwt	r1, -1
	rw	r1, 110

	ib	110
	hlt	040
	ib	110
	hlt	077
	hlt	040

; XPCT rz[6] : 0
; XPCT sr : 0

; XPCT ir : 0xec3f
