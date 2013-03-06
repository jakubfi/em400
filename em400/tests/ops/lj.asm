.program "op/LJ"

	lj label

label:	.data 0
	hlt 077

.endprog

; XPCT int(rz(6)) : 0
; XPCT int(sr) : 0

; XPCT int([2]) : 2

