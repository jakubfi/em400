.prog "op/AWT"

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

.finprog

; XPCT int(rz(6)) : 0
; XPCT int(sr) : 0

; XPCT int(r1) : 30
; XPCT int(r2) : -5
; XPCT int(r3) : 5
; XPCT int(r4) : 53
; XPCT int(r5) : -80
; XPCT int(r6) : -20

