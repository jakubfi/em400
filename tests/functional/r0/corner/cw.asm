	; sets E, clears LG
	lw	r0, 0x5a3c
	cw	r0, 0x5a3c
	rpc	r1

	; sets L, clears EG
	lwt	r0, -1
	cw	r0, 0
	rpc	r2

	; sets G
	lwt	r0, 1
	cw	r0, 0
	rpc	r3

	hlt	077

; XPCT r1 : 0x543c
; XPCT r2 : 0xf9ff
; XPCT r3 : 0x0201
