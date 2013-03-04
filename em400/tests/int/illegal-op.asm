.program "int/illegal-op"

.data	1
	hlt 077

.endprog

; XPCT int(sr) : 0

; XPCT int(rz(6)) : 1

