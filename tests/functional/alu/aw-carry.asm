; -1 + 32767

	lw	r1, 0b1111111111111111
	aw	r1, 0b0111111111111111
	hlt	077

; XPCT rz[6] : 0
; XPCT sr : 0

; XPCT r1 : 32766
; XPCT Z : 0
; XPCT M : 0
; XPCT V : 0
; XPCT C : 1
