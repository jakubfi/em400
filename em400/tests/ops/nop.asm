.prog "op/NOP"

	nop

	hlt 077

.finprog

; XPCT int(rz[6]) : 0
; XPCT int(sr) : 0

; XPCT int(ic): 2
; XPCT oct(ir[10-15]) : 077

