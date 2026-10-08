	; bb should skip when all mask bits are set in register
	; bb should not touch flags
	lwt	r0, -1
	lw	r1, 0b0101101000111100
	bb	r1, 0b0001101000001100
	hlt	040
	rpc	r2

	; bb should not skip when some mask bits are not set in register
	; bb should not set any flags
	lwt	r0, 0
	lwt	r3, -1
	bb	r1, 0b0001101000001101
	rpc	r3

	; bb should always skip for an empty mask
	lwt	r4, 0
	bb	r4, 0
	hlt	040

	hlt	077

; XPCT r1 : 0b0101101000111100
; XPCT r2 : 0xffff
; XPCT r3 : 0
