
	; zlb should zero the left byte
	; zlb should not reset any flags
	lwt	r0, -1
	lw	r1, 0x5a3c
	zlb	r1
	rpc	r2

	; zlb should not set any flags (especially Z on zero result)
	lwt	r0, 0
	lw	r3, 0x5a00
	zlb	r3
	rpc	r4

	hlt	077

; XPCT r1 : 0x003c
; XPCT r2 : -1
; XPCT r3 : 0
; XPCT r4 : 0
