	; bn should skip when no mask bits are set in register
	; bn should not touch flags
	lwt	r0, -1
	lw	r1, 0b0101101000111100
	bn	r1, 0b1010010111000011
	hlt	040
	rpc	r2

	; bn should not skip when some mask bits are set in register
	; bn should not set any flags
	lwt	r0, 0
	lwt	r3, -1
	bn	r1, 0b1010010111000100
	rpc	r3

	; bn should always skip for an empty mask
	bn	r1, 0
	hlt	040

	hlt	077

; XPCT r1 : 0b0101101000111100
; XPCT r2 : 0xffff
; XPCT r3 : 0
