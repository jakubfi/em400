
	lw r1, 15
	cw r1, 15
	jn fin
	hlt 077
fin:	hlt 040

; XPCT int(rz[6]) : 0
; XPCT int(sr) : 0

; XPCT oct(ir[10-15]) : 077

