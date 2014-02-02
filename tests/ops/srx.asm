; PRE r0 = 0b0000000010000000

	lw r1, 0b1000000000000001
	srx r1
	rpc r2
	lw r3, r1

	srx r1
	rpc r4
	lw r5, r1

	srx r1

	hlt 077

; XPCT int(rz[6]) : 0
; XPCT int(sr) : 0

; XPCT bin(r3) : 0b1100000000000000
; XPCT bin(r2) : 0b0000000110000000

; XPCT bin(r5) : 0b1110000000000000
; XPCT bin(r4) : 0b0000000010000000

; XPCT bin(r1) : 0b1111000000000000
; XPCT bin(r0) : 0b0000000010000000
