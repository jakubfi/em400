.prog "op/JCS"

	lw r1, 0b1000000000000000
	aw r1, 0b1000000000000000
	jcs fin
	hlt 040
fin:	hlt 077


.finprog

; XPCT int(rz[6]) : 0
; XPCT int(sr) : 0

; XPCT oct(ir[10-15]) : 077

