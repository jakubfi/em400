; -1 + -1

	lw r1, 0b1111111111111111
	aw r1, 0b1111111111111111
	hlt 077

; XPCT rz[6] : 0
; XPCT sr : 0

; XPCT r1 : -2
; XPCT Z : 0
; XPCT M : 1
; XPCT V : 0
; XPCT C : 1
