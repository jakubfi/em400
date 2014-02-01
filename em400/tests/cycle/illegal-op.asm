; illegal instruction resets MOD

	md 1
.word	1
	lw r1, 0
	hlt 077

; XPCT int(sr) : 0
; XPCT int(rz[6]) : 1

; XPCT int(r1) : 0

