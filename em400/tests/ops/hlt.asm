.prog "op/HLT"

; PRE [20] = 0b0000100000000000

	lw r1, fin
	rw r1, 69
	im 20
	hlt 0
loop:	ujs loop

fin:	hlt 077

.finprog

; XPCT int(rz[6]) : 0

; XPCT int(sr) : 0
; XPCT int(rz[5]) : 0
; XPCT int(ic) : 9

