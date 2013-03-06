.program "op/SRY"

; PRE r0 = 0b0000000100000000

	lw r1, 0b1000000000000001
	sry r1
	rpc r2
	lw r3, r1

	sry r1
	rpc r4
	lw r5, r1

	sry r1

	hlt 077

.endprog

; XPCT int(rz(6)) : 0
; XPCT int(sr) : 0

; XPCT bin(r3) : 0b1100000000000000
; XPCT bin(r2) : 0b0000000100000000

; XPCT bin(r5) : 0b1110000000000000
; XPCT bin(r4) : 0b0000000000000000

; XPCT bin(r1) : 0b0111000000000000
; XPCT bin(r0) : 0b0000000000000000
