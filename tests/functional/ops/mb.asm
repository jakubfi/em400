	; mb should load Q/BS/NB from the low 6 bits, ignore the rest, not touch RM or flags
	im	rm
	lwt	r0, -1
	mb	blk1
	rpc	r1

	; mb should replace Q/BS/NB, not merge them
	; mb should read from block 0 (Q=0), not NB (unconfigured)
	mb	blk2
	hlt	077

rm:	.word	0b0101010101000000
blk1:	.word	0b1010101010011111
blk2:	.word	0b1111111111000101

; XPCT r1 : 0xffff
; XPCT sr : 0b0101010101000101
