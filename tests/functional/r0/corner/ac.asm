	; clears C, carry in from r0
	lw	r0, ?C
	ac	r0, 1
	rpc	r1

	; clears C, no carry in
	lw	r0, 0x0fff
	ac	r0, 1
	rpc	r2

	; sets ZC
	lwt	r0, -1
	ac	r0, 0
	rpc	r3

	hlt	077

; XPCT r1 : 0x1002
; XPCT r2 : 0x1000
; XPCT r3 : 0
