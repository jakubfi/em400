
	lw	r0, ?X

	lw	r1, 0b1000000000000010
	slx	r1
	rpc	r2
	lw	r3, r1

	slx	r1
	rpc	r5
	lw	r4, r1

	slx	r1

	hlt	077

; XPCT int(rz[6]) : 0
; XPCT int(sr) : 0

; XPCT bin(r3) : 0b0000000000000101
; XPCT bin(r2) : 0b0000000110000000

; XPCT bin(r4) : 0b0000000000001011
; XPCT bin(r5) : 0b0000000010000000

; XPCT bin(r1) : 0b0000000000010111
; XPCT bin(r0) : 0b0000000010000000
