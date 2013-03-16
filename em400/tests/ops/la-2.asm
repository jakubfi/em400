.prog "op/LA-2levels"

	la [data1]

	hlt 077

data1:	.data	data2
data2:	.data	40, 41, 42, 43, 44, 45, 46

.finprog

; XPCT int(rz(6)) : 0
; XPCT int(sr) : 0

; XPCT int(r1): 40
; XPCT int(r2): 41
; XPCT int(r3): 42
; XPCT int(r4): 43
; XPCT int(r5): 44
; XPCT int(r6): 45
; XPCT int(r7): 46

