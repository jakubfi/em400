	; lf should read from block 0 (Q=0), not NB (unconfigured)
	mb	blk

	; lf should not touch flags or r4
	lwt	r0, -1
	lwt	r4, -1
	lf	fill
	rpc	r5

	; lf should load r1-r3 from consecutive words, using the address computed before r1 is overwritten
	; lf should not set Z on zero, M on negative
	lwt	r0, 0
	lw	r1, data
	lf	r1
	rpc	r6

	hlt	077

blk:	.word	0b001111
fill:	.word	0x1111, 0x1111, 0x1111
data:	.word	0, 0x8000, 0x5a3c

; XPCT rz[2] : 0
; XPCT r5 : 0xffff
; XPCT r4 : 0xffff
; XPCT r1 : 0
; XPCT r2 : 0x8000
; XPCT r3 : 0x5a3c
; XPCT r6 : 0
