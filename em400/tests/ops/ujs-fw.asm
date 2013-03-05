.program "instruction/UJS-fw"

	ujs fin
.res	5
	hlt 077
fin:	hlt 077
.res	5
	hlt 077

.endprog

; XPCT int(rz(6)) : 0
; XPCT int(sr) : 0

; XPCT int(ic): 8

