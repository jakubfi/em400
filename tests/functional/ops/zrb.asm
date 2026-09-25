
	; zrb should zero the right byte
	; zrb should not reset any flags
	lwt	r0, -1
	lw	r1, 0x5a3c
	zrb	r1
	rpc	r2

	; zrb should not set any flags (especially Z on zero result)
	lwt	r0, 0
	lw	r3, 0x003c
	zrb	r3
	rpc	r4

	hlt	077

; XPCT r1 : 0x5a00
; XPCT r2 : -1
; XPCT r3 : 0
; XPCT r4 : 0
