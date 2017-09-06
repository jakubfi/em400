
	lw	r1, 10
	sw	r1, 100

	lw	r2, 10
	sw	r2, -100

	lw	r3, 10
	sw	r3, -5

	lw	r4, -10
	sw	r4, 100

	lw	r5, -100
	sw	r5, 20

	lw	r6, -10
	sw	r6, -10

	hlt	077

; XPCT rz[6] : 0
; XPCT sr : 0

; XPCT r1 : -90
; XPCT r2 : 110
; XPCT r3 : 15
; XPCT r4 : -110
; XPCT r5 : -120
; XPCT r6 : 0
