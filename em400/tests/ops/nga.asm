
	lw r1, -1
	nga r1

	lw r2, 1
	nga r2

	hlt 077

; XPCT int(rz[6]) : 0
; XPCT int(sr) : 0

; XPCT int(r1) : 1
; XPCT int(r2) : -1

