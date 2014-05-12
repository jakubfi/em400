
	lw r1, 10
	aw r1, 100

	lw r2, 10
	aw r2, -100

	lw r3, 10
	aw r3, -5

	lw r4, -10
	aw r4, 100

	lw r5, -100
	aw r5, 20

	lw r6, -10
	aw r6, -10

	hlt 077

; XPCT int(rz[6]) : 0
; XPCT int(sr) : 0

; XPCT int(r1) : 110
; XPCT int(r2) : -90
; XPCT int(r3) : 5
; XPCT int(r4) : 90
; XPCT int(r5) : -80
; XPCT int(r6) : -20

