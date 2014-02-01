; normal argument is a value

	lw r1, 1
	lw r2, 2
	lw r3, r2
	lw r4, r2 - 10
	lw r5, r1 + r4
	lw r6, r1 - 1
	lw r7, r1 - 2

	hlt 077

; XPCT int(rz[6]) : 0
; XPCT int(sr) : 0

; XPCT int(r1): 1
; XPCT int(r2): 2
; XPCT int(r3): 2
; XPCT int(r4): -8
; XPCT int(r5): -7
; XPCT int(r6): 0
; XPCT int(r7): -1

