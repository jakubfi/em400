	; sit should set rz[30] and rz[31], not touch other interrupts
	fi	ints
	sit

	hlt	077

ints:	.word	0x7ffc	; rz[0] is non-maskable, would get served

; XPCT rz : 0x7fff
