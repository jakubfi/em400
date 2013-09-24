.prog "op/SVY"

; PRE r0 = 0b0000000100000000

	lw r1, 0b0010000000000001
	svy r1
	lw r3, r1
	rpc r2

	svy r1
	lw r4, r1
	rpc r5

	svy r1

	hlt 077

.finprog

; XPCT int(rz[6]) : 0
; XPCT int(sr) : 0

; XPCT bin(r3) : 0b0100000000000011
; XPCT bin(r2) : 0b0010000000000000

; XPCT bin(r4) : 0b1000000000000110
; XPCT bin(r5) : 0b0010000000000000

; XPCT bin(r1) : 0b0000000000001100
; XPCT bin(r0) : 0b0010000100000000
