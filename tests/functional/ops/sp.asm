
	sp	ctx
	hlt	040

ctx:	.word	fin_ok, 0xfafa, 0b0110000000000001
fin_ok:
	hlt	077


; new process vector

; XPCT sr : 0b0110000000000001
; XPCT r0 : 0xfafa
