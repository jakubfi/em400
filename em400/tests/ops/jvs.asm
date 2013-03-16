.prog "op/JVS"

	lw r1, 0b0100000000000000
	svz r1
	jvs fin
	hlt 077
fin:	hlt 077


.finprog

; XPCT int(rz(6)) : 0
; XPCT int(sr) : 0

; XPCT int(ic) : 6
; XPCT bin(r0) : 0b0000000000000000

