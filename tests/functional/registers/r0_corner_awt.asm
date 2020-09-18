
	; sets Z
	lwt	r0, 1
	awt	r0, -1
	rpc	r1

	; clears Z
	lwt	r0, 0
	awt	r0, -1
	rpc	r2

	; sets M
	lw	r0, 0x8000
	awt	r0, 1
	rpc	r3

	; clears M
	lw	r0, 0x4000
	awt	r0, 1
	rpc	r4

	; sets V
	lw	r0, 0b0111111111111111
	awt	r0, 1
	rpc	r5

	; sets C
	lw	r0, 0b1111111111111111
	awt	r0, 1
	rpc	r6

	; clears C
	lw	r0, 0b0001111111111111
	awt	r0, 1
	rpc	r7

	hlt	077

; XPCT r1 : 0
; XPCT r2 : 0xffff
; XPCT r3 : 0x8001
; XPCT r4 : 0x4001
; XPCT r5 : 0x8000
; XPCT r6 : 0
; XPCT r7 : 0b0010000000000000
