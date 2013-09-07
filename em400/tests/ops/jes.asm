.prog "op/JES"

	lw r1, 15
	cw r1, 15
	jes fin
	hlt 040
fin:	hlt 077


.finprog

; XPCT int(rz[6]) : 0
; XPCT int(sr) : 0

; XPCT oct(ir[10-15]) : 077

