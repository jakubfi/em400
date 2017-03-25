; PRECMD clock on

	lw	r1, stack
	rw	r1, 0x61
	lw	r1, fin
	rw	r1, 0x40 + 5
	im	mask
loop:	hlt
	ujs	loop

fin:	hlt	077
mask:	.word	0b0000100000000000
stack:

; XPCT rz[6] : 0

; XPCT sr : 0
; XPCT ir : 0xec3f
