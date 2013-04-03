.prog "op/XR"

	lw r1, 0b1010101010101010
	xr r1, 0b1111111100000000

	lw r2, 0
	xr r2, 0

	hlt 077

.finprog

; XPCT int(rz[6]) : 0
; XPCT int(sr) : 0

; XPCT bin(r1) : 0b0101010110101010
; XPCT int(r2) : 0
; XPCT bin(r0) : 0b1000000000000000
