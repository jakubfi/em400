	; sets ZC
	lwt	r0, 5
	sw	r0, 5
	rpc	r1

	; clears Z
	lw	r0, 0x8001
	sw	r0, 1
	rpc	r2

	; x-0 sets C
	lw	r0, 0x0fff
	sw	r0, 0
	rpc	r3

	hlt	077

; XPCT r1 : 0
; XPCT r2 : 0x8000
; XPCT r3 : 0x0fff
