; @nb:addr matches block q*nb, so nb is ignored in system mode.

	mb	srw
	hlt	077

srw:	.word	11	; q=0, nb=11

; XPCT nb : 11
; XPCT ic : 3
; XPCT @0:3 : 1
; XPCT @11:3 : 0
; XPCT @0:2 : 0
