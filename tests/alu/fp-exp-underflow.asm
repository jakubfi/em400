; 0.9999... * 2^-128
; PRE [0xa0] = 0x7fff
; PRE [0xa1] = 0xffff
; PRE [0xa2] = 0xff80
; 2
; PRE [0xa3] = 0x4000
; PRE [0xa4] = 0x0000
; PRE [0xa5] = 0x0002

	lf 0xa0
	df 0xa3

	hlt 077

; XPCT int(rz[6]) : 0
; XPCT int(rz[7]) : 0
; XPCT int(rz[8]) : 1
; XPCT int(rz[9]) : 0
; XPCT int(rz[10]) : 0
; XPCT int(sr) : 0

