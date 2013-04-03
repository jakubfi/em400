.prog "alu/AD-ov-zero-carry"

; -32768 + -32768

	lw r1, 0b1000000000000000
	aw r1, 0b1000000000000000
	hlt 077

.finprog

; XPCT int(rz[6]) : 0
; XPCT int(sr) : 0

; XPCT int(r1) : 0
; Z, M, V, C
; XPCT int(r0[0]) : 1
; XPCT int(r0[1]) : 0
; XPCT int(r0[2]) : 1
; XPCT int(r0[3]) : 1

