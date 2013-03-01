.program "cpu/pre-mod-nofail"

	md 1
	md 1
	md 1
	lw r1, 4

	hlt 077

.endprog

; XPCT int(sr) : 0

; XPCT int(rz(6)) : 0
; XPCT int(r1): 7

# vim: tabstop=4
