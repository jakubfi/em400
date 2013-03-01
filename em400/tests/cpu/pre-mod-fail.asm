.program "cpu/pre-mod-fail"

	md 1
	md 1
	md 1
	md 1
	lw r1, 1

	hlt 077

.endprog

; XPCT int(sr) : 0

; XPCT int(rz(6)) : 1
; XPCT int(r1): 1

