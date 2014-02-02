
	lwt r1, -1
	trb r1, 1
	hlt 040
	hlt 077

; XPCT int(rz[6]) : 0
; XPCT int(sr) : 0

; XPCT oct(ir[10-15]) : 077

