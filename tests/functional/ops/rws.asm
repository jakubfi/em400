	; rws should store to the farthest backward IC-relative address, not touch flags or the register
	lwt	r0, -1
	lw	r1, 0x5a3c
	ujs	start
bw:	.word	0
	.res	61
start:	rws	r1, bw
	rpc	r2

	; rws should store to the farthest forward IC-relative address, not set Z on zero
	lwt	r0, 0
	lwt	r3, 0
	rws	r3, fw
	rpc	r4

	hlt	077
	.res	61
fw:	.word	-1

; XPCT [4] : 0x5a3c
; XPCT r1 : 0x5a3c
; XPCT r2 : 0xffff
; XPCT [134] : 0
; XPCT r3 : 0
; XPCT r4 : 0
