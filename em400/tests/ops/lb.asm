.prog "op/LB"

	lw r1, 0b0000000000000001
	ou r1, 0b0000000000000011
	.data   err, err, ok, err
ok:	mb blk

	lw r1, 0b0101010111001100
	pw r1, 10

	lw r1, 0b1010101010101010
	lb r1, 21
	lw r2, 0b0101010101010101
	lb r2, 20
	hlt 077

err:	hlt 077
blk:    .data 0b0000000000000001


.finprog

; XPCT int(rz(6)) : 0
; XPCT bin(sr) : 0b0000000000000001

; XPCT bin(r1) : 0b1010101011001100
; XPCT bin(r2) : 0b0101010101010101
; XPCT int(ic) : 23

