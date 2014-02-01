; 5/0
; PRE [0xa0] = 0b0111111111111111
; PRE [0xa1] =                   0b1111111111111111
; PRE [0xa2] = 0b0000000000000010

	ld 0xa0
	dw 0xa2
	rd 0xe0
	rw r0, 0xe2

	hlt 077

; XPCT int(rz[6]) : 0
; XPCT int(rz[7]) : 1
; XPCT int(rz[8]) : 0
; XPCT int(rz[9]) : 0
; XPCT int(rz[10]) : 0
; XPCT int(sr) : 0

