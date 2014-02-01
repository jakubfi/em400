
	lw r1, 14
	cw r1, 15
	jl fin
	hlt 040
fin:	hlt 077

; XPCT int(rz[6]) : 0
; XPCT int(sr) : 0

; XPCT oct(ir[10-15]) : 077

