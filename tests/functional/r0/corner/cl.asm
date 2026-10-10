	; sets E, clears LG
	lw	r0, 0x5a3c
	cl	r0, 0x5a3c
	rpc	r1

	; sets G, clears LE
	lwt	r0, -1
	cl	r0, 0
	rpc	r2

	; sets L
	lwt	r0, 1
	cl	r0, 0x8000
	rpc	r3

	hlt	077

; XPCT r1 : 0x543c
; XPCT r2 : 0xf3ff
; XPCT r3 : 0x0801
