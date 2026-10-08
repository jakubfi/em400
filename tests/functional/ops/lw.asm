	; lw should load argument into register, not touch flags
	lwt	r0, -1
	lw	r1, 0b0101101000111100
	rpc	r2

	; lw should not set any flags (especially Z on zero, M on negative)
	lwt	r0, 0
	lw	r3, 0
	rpc	r4
	lw	r5, 0b1000000000000000
	rpc	r6

	; lw should load the whole word into r0 in OS mode
	lw	r0, 0b1010010111000011

	hlt	077

; XPCT r1 : 0b0101101000111100
; XPCT r2 : 0xffff
; XPCT r3 : 0
; XPCT r4 : 0
; XPCT r5 : 0b1000000000000000
; XPCT r6 : 0
; XPCT r0 : 0b1010010111000011
