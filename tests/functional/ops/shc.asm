	; shc should rotate right, not clear flags
	lwt	r0, -1
	lw	r1, 0b1000000000000010
	shc	r1, 1
	rpc	r2

	; shc should not set flags
	lwt	r0, 0
	lwt	r3, -1
	shc	r3, 1
	rpc	r4

	; shc 0 should rotate by full 16 bits
	lw	r5, 0b1101001100000111
	shc	r5, 0

	; shc should wrap multiple bits
	lw	r6, 0b1101001100000111
	shc	r6, 8

	; shc 15 should rotate by the maximum count
	lw	r7, 0b1101001100000111
	shc	r7, 15

	hlt	077

; XPCT r1 : 0b0100000000000001
; XPCT r2 : 0b1111111111111111
; XPCT r3 : 0b1111111111111111
; XPCT r4 : 0b0000000000000000
; XPCT r5 : 0b1101001100000111
; XPCT r6 : 0b0000011111010011
; XPCT r7 : 0b1010011000001111
