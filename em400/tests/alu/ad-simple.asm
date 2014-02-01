; 1 + 1

	lw r1, 1
	aw r1, 1
	hlt 077

; XPCT int(rz[6]) : 0
; XPCT int(sr) : 0

; XPCT int(r1) : 2
; Z, M, V, C
; XPCT int(r0[0]) : 0
; XPCT int(r0[1]) : 0
; XPCT int(r0[2]) : 0
; XPCT int(r0[3]) : 0

