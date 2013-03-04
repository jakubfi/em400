.program "cycle/p"

; if P is set, next instruction is uneffective and IC jumps over

	lwt r1, -1
	trb r1, 1
	hlt 077
	hlt 077

.endprog

; XPCT int(sr) : 0

; XPCT int(ic) : 4

