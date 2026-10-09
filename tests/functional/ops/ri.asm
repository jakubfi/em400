	.include io.inc

	; ri should store the argument at the address in the register and increment the register
	; ri should not touch flags
	lwt	r0, -1
	lw	r1, 100
	ri	r1, 0x5a3c
	rpc	r2

	; 0xffff is not mapped in the default config
	lw	r7, 15\MEM_PAGE | 0\MEM_SEGMENT
	ou	r7, 0\MEM_FRAME | 1\MEM_MODULE | MEM_CFG
	.word	err, err, ok, err
err:	hlt	040

	; ri should not set any flags when the register wraps to 0 (Z, C, V)
ok:	lwt	r0, 0
	lwt	r3, -1
	ri	r3, 0x1234
	rpc	r4

	; ri should take the argument before the register is incremented
	lw	r5, 101
	ri	r5, r5

	; ri r0 overwrites flags
	lw	r0, 102
	ri	r0, 0x1111

	hlt	077

; XPCT [100] : 0x5a3c
; XPCT r1 : 101
; XPCT r2 : 0xffff
; XPCT [0xffff] : 0x1234
; XPCT r3 : 0
; XPCT r4 : 0
; XPCT [101] : 101
; XPCT r5 : 102
; XPCT [102] : 0x1111
; XPCT r0 : 103
