	; cl should set L when register is less than argument (unsigned), clear E and G, not touch other flags
	lwt	r0, -1
	lwt	r1, 1
	cl	r1, -1
	rpc	r2

	; cl should set E when register equals argument, clear L and G, not touch other flags
	lwt	r0, -1
	lw	r1, 0b1000000000000000
	cl	r1, 0b1000000000000000
	rpc	r3

	; cl should set G when register is greater than argument (unsigned), clear L and E, not touch other flags
	lwt	r0, -1
	lw	r1, 0b1000000000000000
	cl	r1, 0b0111111111111111
	rpc	r4

	; cl should not set any other flags
	lwt	r0, 0
	lwt	r1, -1
	cl	r1, 1
	rpc	r5

	lwt	r0, 0
	lw	r1, 0b0111111111111111
	cl	r1, 0b1000000000000000
	rpc	r6

	lwt	r0, 0
	lwt	r1, 0
	cl	r1, 0
	rpc	r7

	hlt	077

; XPCT r1 : 0
; XPCT r2 : 0b1111100111111111
; XPCT r3 : 0b1111010111111111
; XPCT r4 : 0b1111001111111111
; XPCT r5 : 0b0000001000000000
; XPCT r6 : 0b0000100000000000
; XPCT r7 : 0b0000010000000000
