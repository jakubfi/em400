.prog "op/UJS-bw"

	ujs fin
.res	4
	hlt 077
fin2:	hlt 077
.res	5
	hlt 077
fin:	ujs fin2
.res	5
	hlt 077

.finprog

; XPCT int(rz[6]) : 0
; XPCT int(sr) : 0

; XPCT int(ic): 7

