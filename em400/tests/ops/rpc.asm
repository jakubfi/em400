.program "op/RPC"

	lw r0, 0b0000000001010101
	lw r1, -1
	aw r1, 1
	rpc r5
	hlt 077

.endprog

; XPCT int(rz(6)) : 0
; XPCT int(sr) : 0

; XPCT bin(r0) : 0b1001000001010101
; XPCT bin(r5) : 0b1001000001010101

