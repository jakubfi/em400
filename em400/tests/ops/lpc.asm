.program "op/LPC"

	lw r1, 0b1100110001010101
	lpc r1
	hlt 077

.endprog

; XPCT int(rz(6)) : 0
; XPCT int(sr) : 0

; XPCT bin(r0) : 0b1100110001010101

