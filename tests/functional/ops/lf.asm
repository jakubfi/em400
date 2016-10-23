
	lf data1
	lw r4, r1
	lw r5, r2
	lw r6, r3
	lf [data2]

	hlt 077

data1:	.word	1024, 1025, 1026
data2:	.word	data3
data3:	.word	2048, 2049, 2050

; XPCT rz[6] : 0
; XPCT sr : 0

; XPCT r1 : 2048
; XPCT r2 : 2049
; XPCT r3 : 2050
; XPCT r4 : 1024
; XPCT r5 : 1025
; XPCT r6 : 1026
