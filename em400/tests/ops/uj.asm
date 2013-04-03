.prog "op/UJ"

	lw r1, label
	uj r1
	hlt 077
	hlt 077
	hlt 077
label:	hlt 077
	hlt 077
	hlt 077
	hlt 077

.finprog

; XPCT int(rz[6]) : 0
; XPCT int(sr) : 0

; XPCT int(ic) : 7

