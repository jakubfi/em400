.prog "op/JLS"

	lw r1, 14
	cw r1, 15
	jls fin
	hlt 077
fin:	hlt 077


.finprog

; XPCT int(rz(6)) : 0
; XPCT int(sr) : 0

; XPCT int(ic) : 7

