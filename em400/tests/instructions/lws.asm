.program "instruction/LWS"

	ujs prog
data1:	.data	0x1050
prog:
	lws r1, data1
	lws r2, data2
	ujs fin
data2:	.data	0x1040

fin:	hlt 077

.endprog

; XPCT int(rz(6)) : 0
; XPCT int(sr) : 0

; XPCT hex(r1): 0x1050
; XPCT hex(r2): 0x1040

