.prog "op/JE"

	lw r1, 15
	cw r1, 15
	je fin
	hlt 077
fin:	hlt 077


.finprog

; XPCT int(rz[6]) : 0
; XPCT int(sr) : 0

; XPCT int(ic) : 8

