.prog "cycle/ic-add-1"

; 1-word instruction

	hlt 077

.finprog

; XPCT int(sr) : 0
; XPCT int(rz(6)) : 0

; XPCT int(ic) : 1

