
	lw	r0, 0
	lwt	r1, 0
	lwt	r2, 1
	sd	zero
	hlt	077
zero:	.dword	0


; XPCT rz[6] : 0
; XPCT sr : 0

; XPCT r1 : 0
; XPCT r2 : 1
; XPCT Z : 0
; XPCT M : 0
; XPCT V : 0
; XPCT C : 1
