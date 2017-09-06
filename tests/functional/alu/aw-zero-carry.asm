; -1 + 1

	lw	r1, 0b1111111111111111
	aw	r1, 0b0000000000000001
	hlt	077

; XPCT rz[6] : 0
; XPCT sr : 0

; XPCT r1 : 0
; XPCT Z : 1
; XPCT M : 0
; XPCT V : 0
; XPCT C : 1
