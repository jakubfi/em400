.program "alu/AD-minus-carry"

; -1 + -1

	lw r1, 0b1111111111111111
	aw r1, 0b1111111111111111
	hlt 077

.endprog

; XPCT int(rz(6)) : 0
; XPCT int(sr) : 0

; XPCT int(r1) : -2
; Z, M, V, C
; XPCT int(r0[0]) : 0
; XPCT int(r0[1]) : 1
; XPCT int(r0[2]) : 0
; XPCT int(r0[3]) : 1

