.program "instruction/AW-minus-2"

; 0 + -1

	lw r1, 0
	aw r1, -1
	hlt 077

.endprog

; XPCT int(rz(6)) : 0
; XPCT int(sr) : 0

; XPCT int(r1) : -1
; Z, M, V, C
; XPCT int(r0[0]) : 0
; XPCT int(r0[1]) : 1
; XPCT int(r0[2]) : 0
; XPCT int(r0[3]) : 0

