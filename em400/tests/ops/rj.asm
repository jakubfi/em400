.prog "op/RJ"

	rj r1, label

label:	hlt 077

.finprog

; XPCT int(rz[6]) : 0
; XPCT int(sr) : 0

; XPCT int(r1) : 2

