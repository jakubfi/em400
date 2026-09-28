	; rpc should copy r0 to the register
	lw	r0, 0x5a3c
	rpc	r1

	; rpc should not set Z on zero
	lwt	r0, 0
	rpc	r2
	rpc	r3

	hlt	077

; XPCT r1 : 0x5a3c
; XPCT r2 : 0
; XPCT r3 : 0
