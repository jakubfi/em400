	; bs should skip when register equals argument on bits selected by r7
	; bs should not touch flags
	lwt	r0, -1
	lw	r1, 0b0101101000111100
	lw	r7, 0b1111111100000000
	bs	r1, 0b0101101000000000
	hlt	040
	rpc	r2

	; bs should not skip when register differs from argument on bits selected by r7
	; bs should not set any flags
	lwt	r0, 0
	lwt	r3, -1
	bs	r1, 0b0101101100111100
	rpc	r3

	; bs should always skip for an empty r7 mask
	lwt	r7, 0
	bs	r1, 0b1010010111000011
	hlt	040

	hlt	077

; XPCT r1 : 0b0101101000111100
; XPCT r2 : 0xffff
; XPCT r3 : 0
