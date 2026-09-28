	; brc should skip when any selected bit of r0 right byte is clear
	lw	r0, 0b1101011010101101
	brc	0b10101111
	hlt	040

	; brc should not skip when all selected bits are set
	brc	0b10101101
	ujs	nomask
	hlt	040

	; brc should never skip with no bits selected
nomask:	lwt	r0, 0
	brc	0
	ujs	left
	hlt	040

	; brc should ignore r0 left byte
left:	lw	r0, 0xff00
	brc	0x01
	hlt	040
	lw	r0, 0x00ff
	brc	0xff
	ujs	flags
	hlt	040

	; brc should not modify r0
flags:	lwt	r0, -1
	brc	0xff
	hlt	077
	hlt	040

; XPCT r0 : 0xffff
