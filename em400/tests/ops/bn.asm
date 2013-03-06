.program "op/BN"

	lw r1, 0b1010101010101010
	bn r1, 0b0101010101010101
	hlt 077
	bn r1, 0b0101010101010111
	hlt 077
	hlt 077

.endprog

; XPCT int(rz(6)) : 0
; XPCT int(sr) : 0

; XPCT int(ic) : 8

