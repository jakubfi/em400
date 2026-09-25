
	lwt	r1, -2
	irb	r1, next
	hlt	040
next:	lwt	r2, -1
	irb	r2, fail
	hlt	077
fail:	hlt	040


; XPCT r1 : -1
; XPCT r2 : 0
