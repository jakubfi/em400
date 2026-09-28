	; lpc should replace r0 with the register, leaving the register intact
	lwt	r0, -1
	lw	r1, 0x5a3c
	lpc	r1
	rpc	r2

	; lpc should not set Z on zero
	lwt	r3, 0
	lpc	r3
	rpc	r4

	hlt	077

; XPCT r1 : 0x5a3c
; XPCT r2 : 0x5a3c
; XPCT r3 : 0
; XPCT r4 : 0
