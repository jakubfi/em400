; PRECMD int 1
; PRECMD int 11
; PRECMD int 12
; PRECMD int 27
; PRECMD int 28
; PRECMD int 31

	; ki/fi should access block 0 (Q=0), not NB (unconfigured)
	mb	blk

	; ki should store rz[0-11] and rz[28-31], skip channel interrupts, not touch flags
	lwt	r0, -1
	ki	100
	rpc	r1

	; fi should load rz[0-11] and rz[28-31], not touch channel interrupts, not set any flags
	lwt	r0, 0
	fi	ints1
	rpc	r2
	ki	101

	; fi should clear what is not set in its argument
	fi	ints2
	ki	102

	hlt	077

blk:	.word	0b1010	; NB=10
ints1:	.word	0x5a3c	; rz[0] is non-maskable, would get served
ints2:	.word	0x25c3

; XPCT [100] : 0x4019
; XPCT r1 : 0xffff
; XPCT r2 : 0
; XPCT [101] : 0x5a3c
; XPCT [102] : 0x25c3
; XPCT rz : 0x25c3
; XPCT rz[12] : 1
; XPCT rz[27] : 1
