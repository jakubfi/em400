	; rj should store the address of the next instruction and jump
	rj	r1, l1
	hlt	040

	; jump address is taken before the register is written
l1:	lw	r2, l2
	rj	r2, r2
	hlt	040

	; rj should not touch flags
l2:	lwt	r0, -1
	rj	r3, l3
	hlt	040
l3:	rpc	r4

	; rj r0 overwrites flags
	rj	r0, l4
	hlt	040
l4:	hlt	077

; XPCT r1 : 2
; XPCT r2 : 6
; XPCT r3 : 10
; XPCT r4 : 0xffff
; XPCT r0 : 14
