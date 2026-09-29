	; sil should set rz[31], not touch other interrupts
	fi	in1
	sil
	ki	100

	; sil should not set any other interrupt
	fi	in2
	sil

	hlt	077

in1:	.word	0x7ffe	; rz[0] is non-maskable, would get served
in2:	.word	0

; XPCT [100] : 0x7fff
; XPCT rz : 0x0001
