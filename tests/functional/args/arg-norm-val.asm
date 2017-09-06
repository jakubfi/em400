; normal argument is a value

	lw	r1, 1
	lw	r2, 2
	lw	r3, r2
	lw	r4, r2 - 10
	lw	r5, r1 + r4
	lw	r6, r1 - 1
	lw	r7, r1 - 2

	hlt	077

; XPCT rz[6] : 0
; XPCT sr : 0

; XPCT r1 : 1
; XPCT r2 : 2
; XPCT r3 : 2
; XPCT r4 : -8
; XPCT r5 : -7
; XPCT r6 : 0
; XPCT r7 : -1
