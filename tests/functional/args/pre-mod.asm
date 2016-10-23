; pre-modification

	md 100
	lw r1, 1
	md -1
	lw r2, 1
	md -16
	lw r3, 0
	md -1024
	lw r4, -1024
	md -512
	lw r5, 1024

	hlt 077

; XPCT rz[6] : 0
; XPCT sr : 0

; XPCT r1 : 101
; XPCT r2 : 0
; XPCT r3 : -16
; XPCT r4 : -2048
; XPCT r5 : 512
