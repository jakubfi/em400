	; ta should not touch flags
	lwt	r0, -1
	ta	fill
	rw	r0, 200

	; ta should load r1-r7 from consecutive words, using the address computed before r1 is overwritten
	; ta should not set Z on zero, M on negative
	lwt	r0, 0
	lw	r1, data
	ta	r1

	hlt	077

fill:	.word	0x1111, 0x1111, 0x1111, 0x1111, 0x1111, 0x1111, 0x1111
data:	.word	0, 0x8000, 0x5a3c, 0xa5c3, 0x0f0f, 0xf0f0, 0x00ff

; XPCT [200] : 0xffff
; XPCT r1 : 0
; XPCT r2 : 0x8000
; XPCT r3 : 0x5a3c
; XPCT r4 : 0xa5c3
; XPCT r5 : 0x0f0f
; XPCT r6 : 0xf0f0
; XPCT r7 : 0x00ff
; XPCT r0 : 0
