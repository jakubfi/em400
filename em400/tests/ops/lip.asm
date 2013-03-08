.program "op/LIP"

; PRE [10] = 135
; PRE [11] = 0xfafa
; PRE [12] = 0b1100000000000001

	uj start
	.res 128
start:
	lw r1, 14
	rw r1, 97

	lip
	hlt 077

exlp:
	hlt 077

.endprog

; XPCT int(rz(6)) : 0

; new process vector

; XPCT bin(sr) : 0b1100000000000001
; XPCT hex(r0) : 0xfafa
; XPCT int(ic) : 136

; XPCT int([97]) : 10

