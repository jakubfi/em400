.prog "op/ZRB"

	lw r1, 0xfafa
	zrb r1

	hlt 077

.finprog

; XPCT int(rz[6]) : 0
; XPCT int(sr) : 0

; XPCT hex(r1) : 0xfa00

