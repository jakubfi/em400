
	lw r1, 0b1000000000000001
	srz r1
	rpc r2
	lw r3, r1

	srz r1

	hlt 077

; XPCT int(rz[6]) : 0
; XPCT int(sr) : 0

; XPCT bin(r3) : 0b0100000000000000
; XPCT bin(r2) : 0b0000000100000000

; XPCT bin(r1) : 0b0010000000000000
; XPCT bin(r0) : 0b0000000000000000

