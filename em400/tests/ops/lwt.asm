.program "op/LWT"

	lwt r1, 0
	lwt r2, 63
	lwt r3, -63

	hlt 077

.endprog

; XPCT int(rz(6)) : 0
; XPCT int(sr) : 0

; XPCT int(r1): 0
; XPCT int(r2): 63
; XPCT int(r3): -63

