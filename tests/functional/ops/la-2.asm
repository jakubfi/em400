
	la	[data1]

	hlt	077

data1:	.word	data2
data2:	.word	40, 41, 42, 43, 44, 45, 46

; XPCT rz[6] : 0
; XPCT sr : 0

; XPCT r1 : 40
; XPCT r2 : 41
; XPCT r3 : 42
; XPCT r4 : 43
; XPCT r5 : 44
; XPCT r6 : 45
; XPCT r7 : 46
