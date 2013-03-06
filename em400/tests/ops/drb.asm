.program "op/DRB"

	lwt r1, 2
	drb r1, fin
	hlt 077
fin:	hlt 077

.endprog

; XPCT int(rz(6)) : 0
; XPCT int(sr) : 0

; XPCT int(ic) : 4

