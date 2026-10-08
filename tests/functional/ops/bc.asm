	; bc should skip when some mask bits are not set in register
	; bc should not touch flags
	lwt	r0, -1
	lw	r1, 0b0101101000111100
	bc	r1, 0b0001101000001101
	hlt	040
	rpc	r2

	; bc should not skip when all mask bits are set in register
	; bc should not set any flags
	lwt	r0, 0
	lwt	r3, -1
	bc	r1, 0b0001101000001100
	rpc	r3

	; bc should never skip for an empty mask
	lwt	r4, 0
	bc	r4, 0
	lwt	r5, 1

	hlt	077

; XPCT r1 : 0b0101101000111100
; XPCT r2 : 0xffff
; XPCT r3 : 0
; XPCT r5 : 1
