.prog "op/JXS"

	lw r0, 0b0000000010000000
	jxs fin
	hlt 040
fin:	hlt 077


.finprog

; XPCT int(rz[6]) : 0
; XPCT int(sr) : 0

; XPCT oct(ir[10-15]) : 077

