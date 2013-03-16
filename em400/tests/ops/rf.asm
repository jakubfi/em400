.prog "op/RF"

; PRE r1 = 10
; PRE r2 = 20
; PRE r3 = 30

	rf data

	hlt 077

data:	.res 3

.finprog

; XPCT int(rz(6)) : 0
; XPCT int(sr) : 0

; XPCT int([3]) : 10
; XPCT int([4]) : 20
; XPCT int([5]) : 30

