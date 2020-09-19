
	lwt	r1, 3
	lw	r0, ?V
	sw	r1, 1
	hlt	077

; XPCT rz[6] : 0
; XPCT sr : 0

; XPCT r1 : 2
; XPCT Z : 0
; XPCT M : 0
; XPCT V : 1
; XPCT C : 1
