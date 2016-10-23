
	ll data1
	lw r1, r5
	lw r2, r6
	lw r3, r7
	ll [data2]

	hlt 077

data1:	.word	98, 99, 100
data2:	.word	data3
data3:	.word	101, 102, 103

; XPCT rz[6] : 0
; XPCT sr : 0

; XPCT r5 : 101
; XPCT r6 : 102
; XPCT r7 : 103

; XPCT r1 : 98
; XPCT r2 : 99
; XPCT r3 : 100
