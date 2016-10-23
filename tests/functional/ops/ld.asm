
	ld data1
	lw r3, r1
	lw r4, r2
	ld [data2]

	hlt 077

data1:	.word	12, 13
data2:	.word	data3
data3:	.word	14, 15

; XPCT rz[6] : 0
; XPCT sr : 0

; XPCT r1 : 14
; XPCT r2 : 15
; XPCT r3 : 12
; XPCT r4 : 13
