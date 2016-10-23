; illegal instruction resets MOD

	md 1
.word	1
	lw r1, 0
	hlt 077

; XPCT sr : 0
; XPCT rz[6] : 1

; XPCT r1 : 0
