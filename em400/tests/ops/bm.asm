.prog "op/BM"

	lw r1, 0b0000000000000001
	ou r1, 0b0000000000000011
	.data   err, err, ok, err
ok:
	mb blk

	lw r1, 0b1010101011111111
	pw r1, 30

	lw r1, 0b1010101010101010
	bm r1, 30
	hlt 077
	lw r1, 0b1111101010101010
	bm r1, 30
	hlt 077
	hlt 077

err:	hlt 077
blk:	.data 1

.finprog

; XPCT int(rz[6]) : 0
; XPCT int(sr) : 1

; XPCT int(ic) : 24
