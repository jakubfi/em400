.prog "alu/fp-NRF-zero"

; 0
; PRE [0xa0] = 0x0000
; PRE [0xa1] = 0x0000
; PRE [0xa2] = 0x0000

	lf 0xa0
	nrf

	hlt 077

.finprog

; XPCT int(rz[6]) : 0
; XPCT int(rz[7]) : 0
; XPCT int(rz[8]) : 0
; XPCT int(rz[9]) : 0
; XPCT int(rz[10]) : 0

; Z, M, V, C
; XPCT int(r0[0]) : 1
; XPCT int(r0[1]) : 0
; XPCT int(r0[2]) : 0
; XPCT int(r0[3]) : 0

; 0
; XPCT hex(r1) : 0x0000
; XPCT hex(r2) : 0x0000
; XPCT hex(r3) : 0x0000

; XPCT int(sr) : 0
