.program "cycle/ic-add-2"

; 2-words instruction

	lw r1, 10000

	hlt 077

.endprog

; XPCT int(sr) : 0
; XPCT int(rz(6)) : 0

; XPCT int(ic) : 3

