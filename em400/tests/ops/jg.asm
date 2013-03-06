.program "op/JG"

	lw r1, 16
	cw r1, 15
	jg fin
	hlt 077
fin:	hlt 077


.endprog

; XPCT int(rz(6)) : 0
; XPCT int(sr) : 0

; XPCT int(ic) : 8

