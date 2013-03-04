.program "instruction/AW-ov-carry"

; -1 + -32768

	lw r1, 0b1111111111111111
	aw r1, 0b1000000000000000
	hlt 077

.endprog

; XPCT int(rz(6)) : 0
; XPCT int(sr) : 0

; XPCT int(r1) : 32767
; Z, M, V, C
; XPCT int(r0[0]) : 0
; XPCT int(r0[1]) : 0
; XPCT int(r0[2]) : 1
; XPCT int(r0[3]) : 1

