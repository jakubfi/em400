.program "op/LL"

	ll data1
	lw r1, r5
	lw r2, r6
	lw r3, r7
	ll [data2]

	hlt 077

data1:	.data	98, 99, 100
data2:	.data 	data3
data3:	.data	101, 102, 103

.endprog

; XPCT int(rz(6)) : 0
; XPCT int(sr) : 0

; XPCT int(r5): 101
; XPCT int(r6): 102
; XPCT int(r7): 103

; XPCT int(r1): 98
; XPCT int(r2): 99
; XPCT int(r3): 100

