	; blc should skip when any selected bit of r0 left byte is clear
	lw	r0, 0b1101011010101101
	blc	0b1101011100000000
	hlt	040

	; blc should not skip when all selected bits are set
	blc	0b1101011000000000
	ujs	nomask
	hlt	040

	; blc should never skip with no bits selected
nomask:	lwt	r0, 0
	blc	0
	ujs	right
	hlt	040

	; blc should ignore r0 right byte
right:	lw	r0, 0x00ff
	blc	0x0100
	hlt	040
	lw	r0, 0xff00
	blc	0xff00
	ujs	flags
	hlt	040

	; blc should not modify r0
flags:	lwt	r0, -1
	blc	0xff00
	hlt	077
	hlt	040

; XPCT r0 : 0xffff
