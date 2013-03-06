.program "op/JZ"

	lw r1, 14
	nr r1, 1
	jz fin
	hlt 077
fin:	hlt 077


.endprog

; XPCT int(rz(6)) : 0
; XPCT int(sr) : 0

; XPCT int(ic) : 8

