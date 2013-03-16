.prog "op/BRC"

; PRE r0 = 0b1101011010101101

	brc 0b10101111
	hlt 077
	brc 0b10101101
	hlt 077
	hlt 077

.finprog

; XPCT int(rz(6)) : 0
; XPCT int(sr) : 0

; XPCT int(ic) : 4

