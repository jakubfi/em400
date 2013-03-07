.program "op/RB"

	lw r1, 0b0001100010101010
	rb r1, 20

	lw r2, 0b0001100011001100
	rb r2, 21

	hlt 077

.endprog

; XPCT int(rz(6)) : 0
; XPCT int(sr) : 0

; XPCT bin([10]) : 0b1010101011001100

