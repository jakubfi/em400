.prog "op/IB"

; PRE [10] = -1

	ib 10
	hlt 077
	ib 10
	hlt 077
	hlt 077

.finprog

; XPCT int(rz[6]) : 0
; XPCT int(sr) : 0

; XPCT int(ic) : 6

