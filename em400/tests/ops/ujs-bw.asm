.prog "op/UJS-bw"

	ujs fin
.res	4
	hlt 040
fin2:	hlt 077
.res	5
	hlt 040
fin:	ujs fin2
.res	5
	hlt 040

.finprog

; XPCT int(rz[6]) : 0
; XPCT int(sr) : 0

; XPCT oct(ir[10-15]) : 077

