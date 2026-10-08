	; lwt should load argument into register, not touch flags
	lwt	r0, -1
	lwt	r1, 63
	rpc	r2

	; lwt should not set any flags (especially Z on zero, M on negative)
	lwt	r0, 0
	lwt	r3, 0
	rpc	r4
	lwt	r5, -63
	rpc	r6

	; lwt should load the whole sign-extended word into r0 in OS mode
	lwt	r0, -63

	hlt	077

; XPCT r1 : 0b0000000000111111
; XPCT r2 : 0xffff
; XPCT r3 : 0
; XPCT r4 : 0
; XPCT r5 : 0b1111111111000001
; XPCT r6 : 0
; XPCT r0 : 0b1111111111000001
