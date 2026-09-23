
	lwt	r1, -2
	irb	r1, next
	hlt	040
next:	lwt	r2, -1
	irb	r2, fail
	hlt	077
fail:	hlt	040

; XPCT rz[6] : 0
; XPCT sr : 0

; XPCT ir : 0xec3f
; XPCT r1 : -1
; XPCT r2 : 0
