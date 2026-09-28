	; lws should load from the farthest backward IC-relative address, not touch flags
	lwt	r0, -1
	ujs	start
bw:	.word	0x5a3c
	.res	61
start:	lws	r1, bw
	rpc	r2

	; lws should load from the farthest forward IC-relative address, not set Z on zero
	lwt	r0, 0
	lwt	r3, -1
	lws	r3, fw
	rpc	r4

	hlt	077
	.res	61
fw:	.word	0

; XPCT r1 : 0x5a3c
; XPCT r2 : 0xffff
; XPCT r3 : 0
; XPCT r4 : 0
