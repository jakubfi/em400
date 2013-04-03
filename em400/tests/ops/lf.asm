.prog "op/LF"

	lf data1
	lw r4, r1
	lw r5, r2
	lw r6, r3
	lf [data2]

	hlt 077

data1:	.data	1024, 1025, 1026
data2:	.data	data3
data3:	.data	2048, 2049, 2050

.finprog

; XPCT int(rz[6]) : 0
; XPCT int(sr) : 0

; XPCT int(r1): 2048
; XPCT int(r2): 2049
; XPCT int(r3): 2050
; XPCT int(r4): 1024
; XPCT int(r5): 1025
; XPCT int(r6): 1026

