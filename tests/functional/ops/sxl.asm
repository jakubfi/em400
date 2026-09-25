	; sxl should set X from bit 15 (LSB), not touch other flags or the register
	lw	r0, ~?X
	lwt	r1, 1
	sxl	r1
	rpc	r2

	; sxl should clear X, not touch other flags or the register
	lwt	r0, -1
	lwt	r3, -2
	sxl	r3
	rpc	r4

	hlt	077

; XPCT r1 : 1
; XPCT r2 : 0xffff
; XPCT r3 : 0xfffe
; XPCT r4 : 0xff7f
