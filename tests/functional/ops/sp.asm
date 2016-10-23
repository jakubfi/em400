
	sp ctx
	hlt 040

ctx:	.word fin_ok, 0xfafa, 0b0110000000000001
fin_ok:
	hlt 077

; XPCT rz[6] : 0

; new process vector

; XPCT sr : 0b0110000000000001
; XPCT r0 : 0xfafa
; XPCT ir&0x3f : 0o77
