	; pd should store r1-r2 at consecutive addresses, not touch flags or neighbouring words
	lwt	r0, -1
	lw	r1, 0x5a3c
	lw	r2, 0xa5c3
	pd	101
	rpc	r3

	; pd should not set Z on zero, M on negative
	lwt	r0, 0
	lwt	r1, 0
	lw	r2, 0x8000
	pd	201
	rpc	r4

	; pd should store the register used as the argument, not touch registers
	lw	r1, 301
	pd	r1

	hlt	077

	.org	100
	.word	-1, 0x1111, 0x1111, -1
	.org	201
	.word	-1, -1
	.org	301
	.word	-1, -1

; XPCT [100] : 0xffff
; XPCT [101] : 0x5a3c
; XPCT [102] : 0xa5c3
; XPCT [103] : 0xffff
; XPCT r3 : 0xffff
; XPCT [201] : 0
; XPCT [202] : 0x8000
; XPCT r4 : 0
; XPCT [301] : 301
; XPCT [302] : 0x8000
; XPCT r1 : 301
; XPCT r2 : 0x8000
