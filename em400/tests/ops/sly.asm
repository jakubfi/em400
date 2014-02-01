; PRE r0 = 0b0000000100000000

	lw r1, 0b1000000000000010
	sly r1
	rpc r2
	lw r3, r1

	sly r1
	rpc r5
	lw r4, r1

	sly r1

	hlt 077

; XPCT int(rz[6]) : 0
; XPCT int(sr) : 0

; XPCT bin(r3) : 0b0000000000000101
; XPCT bin(r2) : 0b0000000100000000

; XPCT bin(r4) : 0b0000000000001011
; XPCT bin(r5) : 0b0000000000000000

; XPCT bin(r1) : 0b0000000000010110
; XPCT bin(r0) : 0b0000000000000000

