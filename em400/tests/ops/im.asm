.program "op/IM"

; PRE [10] = 0b1111011111111111

	im 10
	hlt 077

.endprog

; XPCT int(rz(6)) : 0

; XPCT bin([10]) : 0b1111011111111111
; XPCT bin(sr) : 0b1111011111000000


