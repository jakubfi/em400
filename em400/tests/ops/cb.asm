.program "op/CB"

; PRE [40] = 0b1111100000000001

	lw r1, 0b0000000000000010
	cb r1, 80
	rpc r5

	lw r2, 0b0000000000000010
	cb r2, 81
	rpc r6

	lw r3, 0b0000000011111000
	cb r3, 80
	rpc r7

	lw r4, 0b0000000000000001
	cb r4, 81

	hlt 077

.endprog

; XPCT int(rz(6)) : 0
; XPCT int(sr) : 0

; XPCT bin(r5) : 0b0000100000000000
; XPCT bin(r6) : 0b0000001000000000
; XPCT bin(r7) : 0b0000010000000000
; XPCT bin(r0) : 0b0000010000000000

