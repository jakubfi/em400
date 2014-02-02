
	sp ctx
	hlt 040

ctx:	.word fin_ok, 0xfafa, 0b0110000000000001
fin_ok:
	hlt 077

; XPCT int(rz[6]) : 0

; new process vector

; XPCT bin(sr) : 0b0110000000000001
; XPCT hex(r0) : 0xfafa
; XPCT oct(ir[10-15]) : 077

