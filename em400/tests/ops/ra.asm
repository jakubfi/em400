.program "op/RA"

; PRE r1 = 10
; PRE r2 = 20
; PRE r3 = 30
; PRE r4 = 40
; PRE r5 = 50
; PRE r6 = 60
; PRE r7 = 70

	ra data

	hlt 077

data:	.res 7

.endprog

; XPCT int(rz(6)) : 0
; XPCT int(sr) : 0

; XPCT int([3]) : 10
; XPCT int([4]) : 20
; XPCT int([5]) : 30
; XPCT int([6]) : 40
; XPCT int([7]) : 50
; XPCT int([8]) : 60
; XPCT int([9]) : 70

