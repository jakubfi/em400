
	ujs main
data1:	.word	0xffff
	.word	0x1051
	.word	0x1052
	.word	0x1053
main:	lws r0, data1
	lws r1, data1+1
	lws r2, data1+2
	lws r3, data1+3
	lws r4, data2
	lws r5, data2+1
	lws r6, data2+2
	lws r7, data2+3
	ujs fin
data2:	.word	0x1040
	.word	0x1041
	.word	0x1042
	.word	0x1043

fin:	hlt 077

; XPCT rz[6] : 0
; XPCT sr : 0

; XPCT r0 : 0xffff
; XPCT r1 : 0x1051
; XPCT r2 : 0x1052
; XPCT r3 : 0x1053
; XPCT r4 : 0x1040
; XPCT r5 : 0x1041
; XPCT r6 : 0x1042
; XPCT r7 : 0x1043
