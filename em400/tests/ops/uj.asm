
	lw r1, label
	uj r1
	hlt 040
label:	hlt 077
	hlt 040
	hlt 040

; XPCT int(rz[6]) : 0
; XPCT int(sr) : 0

; XPCT oct(ir[10-15]) : 077

