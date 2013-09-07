.prog "op/BLC"

; PRE r0 = 0b1101011010101101

	blc 0b1101011100000000
	hlt 040
	blc 0b1101011000000000
	hlt 077
	hlt 040

.finprog

; XPCT int(rz[6]) : 0
; XPCT int(sr) : 0

; XPCT oct(ir[10-15]) : 077

