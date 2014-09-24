
	lwt	r1, -1
	rw	r1, 110

	ib	110
	hlt	040
	ib	110
	hlt	077
	hlt	040

; XPCT int(rz[6]) : 0
; XPCT int(sr) : 0

; XPCT oct(ir[10-15]) : 077
