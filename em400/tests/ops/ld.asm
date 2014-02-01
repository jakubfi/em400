
	ld data1
	lw r3, r1
	lw r4, r2
	ld [data2]

	hlt 077

data1:	.word	12, 13
data2:	.word	data3
data3:	.word	14, 15

; XPCT int(rz[6]) : 0
; XPCT int(sr) : 0

; XPCT int(r1): 14
; XPCT int(r2): 15
; XPCT int(r3): 12
; XPCT int(r4): 13

