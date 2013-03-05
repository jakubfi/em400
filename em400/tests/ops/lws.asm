.program "op/LWS"

	ujs main
data1:	.data	0xffff
	.data	0x1051
	.data	0x1052
	.data	0x1053
main:	lws r0, data1
	lws r1, data1+1
	lws r2, data1+2
	lws r3, data1+3
	lws r4, data2
	lws r5, data2+1
	lws r6, data2+2
	lws r7, data2+3
	ujs fin
data2:	.data	0x1040
	.data	0x1041
	.data	0x1042
	.data	0x1043

fin:	hlt 077

.endprog

; XPCT int(rz(6)) : 0
; XPCT int(sr) : 0

; XPCT hex(r0): 0x00ff
; XPCT hex(r1): 0x1051
; XPCT hex(r2): 0x1052
; XPCT hex(r3): 0x1053
; XPCT hex(r4): 0x1040
; XPCT hex(r5): 0x1041
; XPCT hex(r6): 0x1042
; XPCT hex(r7): 0x1043

