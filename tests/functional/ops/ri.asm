
	rz	11
	lwt	r1, 10
	ri	r1, 15

	hlt	077


; XPCT r1 : 11
; XPCT [10] : 15
; XPCT [11] : 0
