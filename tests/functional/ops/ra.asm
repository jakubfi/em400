
	lw	r1, 10
	lw	r2, 20
	lw	r3, 30
	lw	r4, 40
	lw	r5, 50
	lw	r6, 60
	lw	r7, 70

	ra	data

	hlt	077

	.org	20
data:

; XPCT rz[6] : 0
; XPCT sr : 0

; XPCT [20] : 10
; XPCT [21] : 20
; XPCT [22] : 30
; XPCT [23] : 40
; XPCT [24] : 50
; XPCT [25] : 60
; XPCT [26] : 70
