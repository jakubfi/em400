.prog "op/SLZ"

	lw r1, 0b1000000000000001
	slz r1
	lw r3, r1
	rpc r2

	slz r1

	hlt 077

.finprog

; XPCT int(rz[6]) : 0
; XPCT int(sr) : 0

; XPCT bin(r3) : 0b0000000000000010
; XPCT bin(r2) : 0b0000000100000000

; XPCT bin(r1) : 0b0000000000000100
; XPCT bin(r0) : 0b0000000000000000
