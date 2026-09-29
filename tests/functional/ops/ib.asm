	; ib should increment the memory word, not skip on non-zero result, not touch flags
	lwt	r0, -1
	ib	100
	rpc	r1

	; ib should not set any flags (V on 0x7fff+1, Z/C on 0xffff+1)
	lwt	r0, 0
	lwt	r2, -1
	lwt	r3, -1
	ib	101
	rpc	r2
	ib	102
	hlt	040
	rpc	r3

	hlt	077

	.org	100
	.word	0x5a3c
	.word	0x7fff
	.word	0xffff

; XPCT [100] : 0x5a3d
; XPCT r1 : 0xffff
; XPCT [101] : 0x8000
; XPCT r2 : 0
; XPCT [102] : 0
; XPCT r3 : 0
