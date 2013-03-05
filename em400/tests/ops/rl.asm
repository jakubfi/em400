.program "instruction/RL"

; PRE r5 = 10
; PRE r6 = 20
; PRE r7 = 30

	rl data

	hlt 077

data:	.res 3

.endprog

; XPCT int(rz(6)) : 0
; XPCT int(sr) : 0

; XPCT int([3]) : 10
; XPCT int([4]) : 20
; XPCT int([5]) : 30

