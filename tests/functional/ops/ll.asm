	; ll should read from block 0 (Q=0), not NB (unconfigured)
	mb	blk

	; ll should not touch flags or r1-r4
	lwt	r0, -1
	lwt	r1, -1
	lwt	r2, -1
	lwt	r3, -1
	lwt	r4, -1
	ll	fill
	rw	r0, 200

	; ll should load r5-r7 from consecutive words, using the address computed before r5 is overwritten
	; ll should not set Z on zero, M on negative
	lwt	r0, 0
	lw	r5, data
	ll	r5
	rw	r0, 201

	hlt	077

blk:	.word	0b001111
fill:	.word	0x1111, 0x1111, 0x1111
data:	.word	0, 0x8000, 0x5a3c

; XPCT rz[2] : 0
; XPCT [200] : 0xffff
; XPCT r1 : 0xffff
; XPCT r2 : 0xffff
; XPCT r3 : 0xffff
; XPCT r4 : 0xffff
; XPCT r5 : 0
; XPCT r6 : 0x8000
; XPCT r7 : 0x5a3c
; XPCT [201] : 0
