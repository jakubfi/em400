.prog "op/RIC"

	ric r1
	ric r2
	ric r3
	ric r4

	hlt 077

.finprog

; XPCT int(rz(6)) : 0
; XPCT int(sr) : 0

; XPCT int(r1) : 1
; XPCT int(r2) : 2
; XPCT int(r3) : 3
; XPCT int(r4) : 4

