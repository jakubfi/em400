	lwt	r0, 0
	lwt	r1, 10
	ac	r1, 100

	lw	r0, ?C
	lwt	r2, 10
	ac	r2, 100

	hlt	077

; XPCT r1 : 110
; XPCT r2 : 111
