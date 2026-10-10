	; sets ZC
	lwt	r0, 1
	aw	r0, -1
	rpc	r1

	; clears Z
	lw	r0, 0x8000
	aw	r0, 1
	rpc	r2

	; clears C
	lw	r0, ?C
	aw	r0, 0x0fff
	rpc	r3

	; sets V
	lw	r0, 0x7fff
	aw	r0, 1
	rpc	r4

	hlt	077

; XPCT r1 : 0
; XPCT r2 : 0x8001
; XPCT r3 : 0x1fff
; XPCT r4 : 0x8000
