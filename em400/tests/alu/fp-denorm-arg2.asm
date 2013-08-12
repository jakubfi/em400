.prog "alu/fp-denorm-arg2"

; 4
; PRE [0xa0] = 0x4000
; PRE [0xa1] = 0x0000
; PRE [0xa2] = 0x0003
; 4 (denormalized)
; PRE [0xa3] = 0x2000
; PRE [0xa4] = 0x0000
; PRE [0xa5] = 0x0004

	lf 0xa3
	df 0xa0

	hlt 077

.finprog

; XPCT int(rz[6]) : 0
; XPCT int(rz[7]) : 0
; XPCT int(rz[8]) : 0
; XPCT int(rz[9]) : 0
; XPCT int(rz[10]) : 1
; XPCT int(sr) : 0

