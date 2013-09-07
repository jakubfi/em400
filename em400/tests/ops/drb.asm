.prog "op/DRB"

	lwt r1, 2
	drb r1, fin
	hlt 040
fin:	hlt 077

.finprog

; XPCT int(rz[6]) : 0
; XPCT int(sr) : 0

; XPCT oct(ir[10-15]) : 077

