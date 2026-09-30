	; is should or register into memory and not skip when some register bits are not set in memory
	; is should not touch flags
	lwt	r0, -1
	lw	r1, 0x5a3c
	is	r1, 100
	rpc	r2

	; is should skip and leave memory intact when all register bits are already set in memory
	; is should not set any flags
	lwt	r0, 0
	lw	r3, 0x1a0c
	lwt	r4, -1
	is	r3, 101
	hlt	040
	rpc	r4

	; is should always skip for an empty mask
	lwt	r5, 0
	is	r5, 102
	hlt	040

	hlt	077

	.org	100
	.word	0xff00
	.word	0x5a3c
	.word	0

; XPCT [100] : 0xff3c
; XPCT r1 : 0x5a3c
; XPCT r2 : 0xffff
; XPCT [101] : 0x5a3c
; XPCT r3 : 0x1a0c
; XPCT r4 : 0
; XPCT [102] : 0
