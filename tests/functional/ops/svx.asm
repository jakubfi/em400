	; svx should shift left with X in, set Y from MSB, not clear V, not touch other flags
	lw	r0, ~?Y
	lw	r1, 0b1101010101010101
	svx	r1
	rpc	r2

	; svx should shift in X=0 (not Y), clear Y from MSB, set V on 0->1 sign change
	lw	r0, ?Y
	lw	r3, 0b0101010101010101
	svx	r3
	rpc	r4

	; svx should set V on 1->0 sign change
	lwt	r0, 0
	lw	r5, 0b1010101010101010
	svx	r5
	rpc	r6

	; svx should not set V without sign change
	lwt	r0, 0
	lw	r7, 0b0010101010101010
	svx	r7

	hlt	077

; XPCT r1 : 0b1010101010101011
; XPCT r2 : 0b1111111111111111
; XPCT r3 : 0b1010101010101010
; XPCT r4 : 0b0010000000000000
; XPCT r5 : 0b0101010101010100
; XPCT r6 : 0b0010000100000000
; XPCT r7 : 0b0101010101010100
; XPCT r0 : 0b0000000000000000
