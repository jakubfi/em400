
	lwt	r1, 0
	lw	r0, ?V
	ac	r1, 1
	hlt	077


; XPCT r1 : 1
; XPCT Z : 0
; XPCT M : 0
; XPCT V : 1
; XPCT C : 0
