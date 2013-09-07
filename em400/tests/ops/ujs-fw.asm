.prog "op/UJS-fw"

	ujs fin
.res	5
	hlt 040
fin:	hlt 077
.res	5
	hlt 040

.finprog

; XPCT int(rz[6]) : 0
; XPCT int(sr) : 0

; XPCT oct(ir[10-15]) : 077

