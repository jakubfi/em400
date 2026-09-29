	; ngl should set Z on zero result, not clear other flags
	lw	r0, 0x7fff
	lwt	r1, -1
	ngl	r1
	rpc	r2

	; ngl should clear Z on non-zero result, not set other flags (especially M on negative result)
	lw	r0, ?Z
	lw	r3, 0x5a3c
	ngl	r3
	rpc	r4

	hlt	077

; XPCT r1 : 0
; XPCT r2 : 0xffff
; XPCT r3 : 0xa5c3
; XPCT r4 : 0
