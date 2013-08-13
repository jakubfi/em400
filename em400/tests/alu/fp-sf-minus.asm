.prog "alu/fp-SF-minus"

; 4
; PRE [0xa0] = 0x4000
; PRE [0xa1] = 0x0000
; PRE [0xa2] = 0x0003
; 7
; PRE [0xa3] = 0x7000
; PRE [0xa4] = 0x0000
; PRE [0xa5] = 0x0003

	lf 0xa0
	sf 0xa3

	hlt 077

.finprog

; XPCT int(rz[6]) : 0
; XPCT int(rz[7]) : 0
; XPCT int(rz[8]) : 0
; XPCT int(rz[9]) : 0
; XPCT int(rz[10]) : 0
; XPCT int(sr) : 0

; Z, M, V
; XPCT int(r0[0]) : 0
; XPCT int(r0[1]) : 1
; XPCT int(r0[2]) : 0

; -3
; XPCT hex(r1) : 0xa000
; XPCT hex(r2) : 0x0000
; XPCT hex(r3) : 0x0002

