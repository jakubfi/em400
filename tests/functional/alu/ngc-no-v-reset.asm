
	lwt	r1, 1
	lw	r0, ?V
	ngc	r1
	hlt	077

; XPCT rz[6] : 0
; XPCT sr : 0

; XPCT r1 : 0xfffe
; XPCT Z : 0
; XPCT M : 1
; XPCT V : 1
; XPCT C : 0
