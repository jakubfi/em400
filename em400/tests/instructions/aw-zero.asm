.program "instruction/AW-zero"

; 0 + 0

	lw r1, 0
	aw r1, 0
	hlt 077

.endprog

; XPCT int(rz(6)) : 0
; XPCT int(sr) : 0

; XPCT int(r1) : 0
; Z, M, V, C
; XPCT int(r0[0]) : 1
; XPCT int(r0[1]) : 0
; XPCT int(r0[2]) : 0
; XPCT int(r0[3]) : 0

