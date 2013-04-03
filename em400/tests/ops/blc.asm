.prog "op/BLC"

; PRE r0 = 0b1101011010101101

	blc 0b11010111
	hlt 077
	blc 0b11010110
	hlt 077
	hlt 077

.finprog

; XPCT int(rz[6]) : 0
; XPCT int(sr) : 0

; XPCT int(ic) : 4

