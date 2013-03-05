.program "op/AC"

; PRE r0 = 0b0001000000000000

	lw r1, 10
	ac r1, 100

	hlt 077

.endprog

; XPCT int(rz(6)) : 0
; XPCT int(sr) : 0

; XPCT int(r1) : 111

