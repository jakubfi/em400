; pre-modification works 3 times in a row

	md 1
	md 1
	md 1
	lw r1, 4

	hlt 077

; XPCT sr : 0

; XPCT rz[6] : 0
; XPCT r1 : 7
