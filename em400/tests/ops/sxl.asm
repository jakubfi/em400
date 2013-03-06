.program "op/SXL"

	lw r1, -1
	sxl r1
	rpc r2

	lw r1, 0
	sxl r1
	rpc r3

	hlt 077

.endprog

; XPCT int(rz(6)) : 0
; XPCT int(sr) : 0

; XPCT bin(r2) : 0b0000000010000000
; XPCT bin(r3) : 0b1000000000000000

