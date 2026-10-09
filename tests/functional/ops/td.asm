	; td should not touch flags or r3
	lwt	r0, -1
	lwt	r3, -1
	td	fill
	rpc	r4

	; td should load r1-r2 from consecutive words, using the address computed before r1 is overwritten
	; td should not set Z on zero, M on negative
	lwt	r0, 0
	lw	r1, data
	td	r1
	rpc	r5

	hlt	077

fill:	.word	0x1111, 0x1111
data:	.word	0, 0x8000

; XPCT r4 : 0xffff
; XPCT r3 : 0xffff
; XPCT r1 : 0
; XPCT r2 : 0x8000
; XPCT r5 : 0
