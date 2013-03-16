.prog "op/MB"

	mb blk

	hlt 077
blk:	.data 0b1010

.finprog

; XPCT int(rz(6)) : 0
; XPCT bin(sr) : 0b0000000000001010

