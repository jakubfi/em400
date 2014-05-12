
	lw r1, -1
	sxu r1
	rpc r2

	lw r1, 0
	sxu r1
	rpc r3

	hlt 077

; XPCT int(rz[6]) : 0
; XPCT int(sr) : 0

; XPCT bin(r2) : 0b0000000010000000
; XPCT bin(r3) : 0b0000000000000000

