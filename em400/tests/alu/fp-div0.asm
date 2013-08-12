.prog "alu/fp-div0"

; 5
; PRE [0xa0] = 0x5000
; PRE [0xa1] = 0x0000
; PRE [0xa2] = 0x0003
; 0
; PRE [0xa3] = 0x0000
; PRE [0xa4] = 0x0000
; PRE [0xa5] = 0x0000

	lf 0xa0
	df 0xa3

	hlt 077

.finprog

; XPCT int(rz[6]) : 0
; XPCT int(rz[7]) : 0
; XPCT int(rz[8]) : 0
; XPCT int(rz[9]) : 0
; XPCT int(rz[10]) : 1
; XPCT int(sr) : 0

