.prog "op/SIT"

	sit
	hlt 077

.finprog

; XPCT int(rz[6]) : 0
; XPCT int(sr) : 0

; XPCT int(rz[30]) : 1
; XPCT int(rz[31]) : 1

