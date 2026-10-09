	; pw should store the register at the argument address, not touch flags or the register
	lwt	r0, -1
	lw	r1, 0x5a3c
	pw	r1, 100
	rpc	r2

	; pw should not set Z on zero
	lwt	r0, 0
	lwt	r3, 0
	pw	r3, 101
	rpc	r4

	; pw should store the register used as the argument
	lw	r5, 102
	pw	r5, r5

	; pw r0 should store flags
	lw	r0, 0xa5c3
	pw	r0, 103

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
