.prog "op/SVX"

; PRE r0 = 0b0000000010000000

	lw r1, 0b0010000000000001
	svx r1
	lw r3, r1
	rpc r2

	svx r1
	lw r4, r1
	rpc r5

	svx r1

	hlt 077

.finprog

; XPCT int(rz(6)) : 0
; XPCT int(sr) : 0

; XPCT bin(r3) : 0b0100000000000011
; XPCT bin(r2) : 0b0000000010000000

; XPCT bin(r4) : 0b1000000000000111
; XPCT bin(r5) : 0b0010000010000000

; XPCT bin(r1) : 0b0000000000001111
; XPCT bin(r0) : 0b0010000110000000
