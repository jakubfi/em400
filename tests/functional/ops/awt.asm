	lwt	r1, 10
	awt	r1, 20

	lwt	r2, 10
	awt	r2, -20

	hlt	077

; XPCT r1 : 30
; XPCT r2 : -10
