	; tw should load the word at the argument address into register, not touch flags
	lwt	r0, -1
	lwt	r1, 0
	tw	r1, 100
	rpc	r2

	; tw should not set any flags (especially Z on zero, M on negative)
	lwt	r0, 0
	lwt	r3, -1
	tw	r3, 101
	rpc	r4
	lwt	r5, -1
	tw	r5, 102
	rpc	r6

	; tw should load the whole word into r0 in OS mode
	tw	r0, 103

	hlt	077

	.org	100
	.word	0x5a3c, 0, 0x8000, 0xa5c3

; XPCT r1 : 0x5a3c
; XPCT r2 : 0xffff
; XPCT r3 : 0
; XPCT r4 : 0
; XPCT r5 : 0x8000
; XPCT r6 : 0
; XPCT r0 : 0xa5c3
