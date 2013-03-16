.prog "op/OM"

	lw r1, 0b0000000000000001
	ou r1, 0b0000000000000011
	.data   err, err, ok, err
ok:
	mb blk

	lw r1, 0b1111111100000000
	om r1, 10
	lw r1, 0b0000001101010101
	om r1, 10

	lw r2, 0
	om r2, 11

	hlt 077

err:	hlt 077
blk:	.data 1

.finprog

; XPCT int(rz(6)) : 0
; XPCT int(sr) : 1

; XPCT bin([1:10]) : 0b1111111101010101
; XPCT int([1:11]) : 0
; XPCT bin(r0) : 0b1000000000000000
