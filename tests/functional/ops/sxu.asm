	; sxu should set X from bit 0 (MSB), not touch other flags or the register
	lw	r0, ~?X
	lw	r1, 0x8000
	sxu	r1
	rpc	r2

	; sxu should clear X, not touch other flags or the register
	lwt	r0, -1
	lw	r3, 0x7fff
	sxu	r3
	rpc	r4

	hlt	077

; XPCT r1 : 0x8000
; XPCT r2 : 0xffff
; XPCT r3 : 0x7fff
; XPCT r4 : 0xff7f
