	; ric should load IC of the next instruction, not touch flags
	lwt	r0, -1
	uj	start
	.org	0x1234
start:	ric	r1
	rpc	r2

	hlt	077

; XPCT r1 : 0x1235
; XPCT r2 : 0xffff
