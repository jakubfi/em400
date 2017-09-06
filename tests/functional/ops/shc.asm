
	lw	r1, 0b1000000000000001
	shc	r1, 0
	lw	r2, r1

	shc	r1, 1
	lw	r3, r1

	shc	r1, 5
	lw	r4, r1

	shc	r1, 15

	hlt	077

; XPCT rz[6] : 0
; XPCT sr : 0

; XPCT r2 : 0b1000000000000001
; XPCT r3 : 0b1100000000000000
; XPCT r4 : 0b0000011000000000
; XPCT r1 : 0b0000110000000000
