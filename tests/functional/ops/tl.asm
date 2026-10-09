	; tl should not touch flags or r4
	lwt	r0, -1
	lwt	r4, -1
	tl	fill
	rpc	r1

	; tl should load r5-r7 from consecutive words, using the address computed before r5 is overwritten
	; tl should not set Z on zero, M on negative
	lwt	r0, 0
	lw	r5, data
	tl	r5
	rpc	r2

	hlt	077

fill:	.word	0x1111, 0x1111, 0x1111
data:	.word	0, 0x8000, 0x5a3c

; XPCT r1 : 0xffff
; XPCT r4 : 0xffff
; XPCT r5 : 0
; XPCT r6 : 0x8000
; XPCT r7 : 0x5a3c
; XPCT r2 : 0
