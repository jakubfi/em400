
	lwt	r1, -1
	trb	r1, 1
	hlt	040
	lwt	r2, -2
	trb	r2, 1
	ujs	fin
	hlt	040
fin:	hlt	077


; XPCT r1 : 0
; XPCT r2 : -1
