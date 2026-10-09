	; rw should store the register at the argument address, not touch flags or the register
	lwt	r0, -1
	lw	r1, 0x5a3c
	rw	r1, 100
	rpc	r2

	; rw should not set Z on zero
	lwt	r0, 0
	lwt	r3, 0
	rw	r3, 101
	rpc	r4

	; rw should store the register used as the argument
	lw	r5, 102
	rw	r5, r5

	; rw r0 should store flags
	lw	r0, 0xa5c3
	rw	r0, 103

	hlt	077

	.org	101
	.word	-1

; XPCT [100] : 0x5a3c
; XPCT r1 : 0x5a3c
; XPCT r2 : 0xffff
; XPCT [101] : 0
; XPCT r3 : 0
; XPCT r4 : 0
; XPCT [102] : 102
; XPCT r5 : 102
; XPCT [103] : 0xa5c3
