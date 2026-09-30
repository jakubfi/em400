	; im should load RM, not touch Q/BS/NB, not touch flags
	; im should read from block Q (0), not NB (unconfigured)
	lwt	r0, -1
	mb	blk
	im	rm
	rpc	r1
	hlt	077

rm:	.word	0b0101010101111111
blk:	.word	0b1111111111011010

; XPCT sr : 0b0101010101011010
; XPCT r1 : 0xffff
