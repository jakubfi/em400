	; cit should clear rz[30] and rz[31], not touch other interrupts
	fi	ints
	cit

	hlt	077

ints:	.word	0x7fff	; rz[0] is non-maskable, would get served

; XPCT rz : 0x7ffc
