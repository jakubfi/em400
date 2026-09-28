; PRECMD REG KB 0x5a3c

	; rky should load the keys register, not touch flags
	lwt	r0, -1
	rky	r1
	rpc	r2

	hlt	077

; XPCT r1 : 0x5a3c
; XPCT r2 : 0xffff
