.prog "op/SP"

	lw r1, 0b0000000000000001
	ou r1, 0b0000000000000011
	.data   err, err, ok, err
ok:
	mb blk

	lw r1, 21
	lw r2, 0xfafa
	lw r3, 0b0110000000000001
	pf 20

	sp 20

	hlt 077

err:
	hlt 077

blk:	.data 1

.finprog

; XPCT int(rz(6)) : 0

; new process vector

; XPCT bin(sr) : 0b0110000000000001
; XPCT hex(r0) : 0xfafa
; XPCT int(ic) : 22

