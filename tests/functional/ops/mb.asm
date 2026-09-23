	im	rm
	mb	blk
	hlt	077

rm:	.word	0b0101010101000000
blk:	.word	0b1111111111011111

; XPCT rz[6] : 0
; XPCT ir : 0xec3f
; XPCT sr : 0b0101010101011111
