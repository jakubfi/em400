.prog "args/pre-mod-nofail"

; pre-modification works 3 times in a row

	md 1
	md 1
	md 1
	lw r1, 4

	hlt 077

.finprog

; XPCT int(sr) : 0

; XPCT int(rz[6]) : 0
; XPCT int(r1): 7

