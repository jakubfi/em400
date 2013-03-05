.program "op/RD"

; PRE r1 = 10
; PRE r2 = 20

	rd data

	hlt 077

data:	.res 2

.endprog

; XPCT int(rz(6)) : 0
; XPCT int(sr) : 0

; XPCT int([3]) : 10
; XPCT int([4]) : 20

