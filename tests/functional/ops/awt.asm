
	lw r1, 10
	awt r1, 20

	lw r2, 5
	awt r2, -10

	lw r3, 10
	awt r3, -5

	lw r4, -10
	awt r4, 63

	lw r5, -100
	awt r5, 20

	lw r6, -10
	awt r6, -10

	hlt 077

; XPCT rz[6] : 0
; XPCT sr : 0

; XPCT r1 : 30
; XPCT r2 : -5
; XPCT r3 : 5
; XPCT r4 : 53
; XPCT r5 : -80
; XPCT r6 : -20
