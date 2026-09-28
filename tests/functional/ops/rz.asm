
	; rz should zero the memory word, not touch flags
	lwt	r0, -1
	rz	100
	rpc	r1

	; rz should not set Z (or any other flag)
	lwt	r0, 0
	rz	101
	rpc	r2

	hlt	077

	.org	100
	.word	-1
	.word	0xa5c3

; XPCT [100] : 0
; XPCT r1 : 0xffff
; XPCT [101] : 0
; XPCT r2 : 0
