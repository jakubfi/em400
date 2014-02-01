; CONFIG configs/minimal-clock.cfg
; PRE [20] = 0b0000100000000000

	lw r1, fin
	rw r1, 69
	im 20
loop:	hlt 0
	ujs loop

fin:	hlt 077

; XPCT int(rz[6]) : 0

; XPCT int(sr) : 0
; XPCT int(rz[5]) : 0
; XPCT oct(ir[10-15]) : 077

