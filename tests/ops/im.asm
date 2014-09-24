
	lw	r1, 0b1111011111111111
	rw	r1, 20
	im	20
	hlt	077

; XPCT int(rz[6]) : 0

; XPCT bin([20]) : 0b1111011111111111
; XPCT bin(sr) : 0b1111011111000000
