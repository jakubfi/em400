.prog "args/pre-mod-fail"

; pre-modification fails on 4th time

	md 1
	md 1
	md 1
	md 1
	lw r1, 1

	hlt 077

.finprog

; XPCT int(sr) : 0

; XPCT int(rz(6)) : 1
; XPCT int(r1): 1

