
	lw r1, 0b0100000000000000
	svz r1
	jvs fin
	hlt 040
fin:	hlt 077

; XPCT int(rz[6]) : 0
; XPCT int(sr) : 0

; XPCT oct(ir[10-15]) : 077
; XPCT bin(r0) : 0b0000000000000000

