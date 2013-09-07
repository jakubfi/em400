.prog "op/JG"

	lw r1, 16
	cw r1, 15
	jg fin
	hlt 040
fin:	hlt 077


.finprog

; XPCT int(rz[6]) : 0
; XPCT int(sr) : 0

; XPCT oct(ir[10-15]) : 077

