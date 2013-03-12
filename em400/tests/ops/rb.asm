.program "op/RB"

	lw r1, 0b0000000000000001
	ou r1, 0b0000000000000011
	.data   err, err, ok, err
ok:	mb blk

	lw r1, 0b0001100010101010
	rb r1, 20

	lw r2, 0b0001100011001100
	rb r2, 21

	hlt 077

err:	hlt 077
blk:	.data 0b0000000000000001


.endprog

; XPCT int(rz(6)) : 0
; XPCT bin(sr) : 0b0000000000000001

; XPCT bin([1:10]) : 0b1010101011001100
; XPCT int(ic) : 19

