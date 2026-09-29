	; siu should set rz[30], not touch other interrupts
	fi	in1
	siu
	ki	100

	; siu should not set any other interrupt
	fi	in2
	siu

	hlt	077

in1:	.word	0x7ffd	; rz[0] is non-maskable, would get served
in2:	.word	0

; XPCT [100] : 0x7fff
; XPCT rz : 0x0002
