	; nop should only advance IC, not touch flags
	lwt	r0, -1
	nop
	rpc	r1

	hlt	077

; XPCT r1 : 0xffff
