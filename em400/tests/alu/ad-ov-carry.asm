; -1 + -32768

	lw r1, 0b1111111111111111
	aw r1, 0b1000000000000000
	; =    1 0111111111111111
	hlt 077

; XPCT int(rz[6]) : 0
; XPCT int(sr) : 0

; XPCT int(r1) : 32767
; Z, M, V, C
; XPCT int(r0[0]) : 0
; XPCT int(r0[1]) : 1
; XPCT int(r0[2]) : 1
; XPCT int(r0[3]) : 1

