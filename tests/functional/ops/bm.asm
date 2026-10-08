	; bm should skip when all register bits are set in memory
	; bm should not touch flags
	lwt	r0, -1
	lw	r1, 0b0001101000001100
	bm	r1, 100
	hlt	040
	rpc	r2

	; bm should not skip when some register bits are not set in memory
	; bm should not set any flags
	lwt	r0, 0
	lw	r3, 0b0001101000001101
	lwt	r4, -1
	bm	r3, 100
	rpc	r4

	; bm should always skip for an empty register
	lwt	r5, 0
	bm	r5, 101
	hlt	040

	hlt	077

	.org	100
	.word	0b0101101000111100
	.word	0

; XPCT [100] : 0b0101101000111100
; XPCT r1 : 0b0001101000001100
; XPCT r2 : 0xffff
; XPCT r3 : 0b0001101000001101
; XPCT r4 : 0
