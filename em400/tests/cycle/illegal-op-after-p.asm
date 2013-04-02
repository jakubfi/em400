.prog "cycle/illegal-op-after-P"

; illegal instruction after P is not illegal, just skipped

; PRE r1 = 15

	bn r1, 0
	lw r1, 0x0003

	hlt 077

.finprog

; XPCT int(sr) : 0
; XPCT int(rz(6)) : 0

; XPCT int(r1) : 15

