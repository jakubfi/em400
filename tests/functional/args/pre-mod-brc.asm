; pre-modification affects BRC byte argument
; premodified argument is 16-bit, but BRC tests only the right byte of r0
; so bits in the left byte always cause a skip

	lwt	r0, 1
	md	1
	brc	1	; arg = 2
	hlt	040

	lw	r0, 0x00ff
	md	0x3f
	brc	0x40	; arg = 0x7f
	ujs	1
	hlt	041

	lw	r0, 0xffff
	md	0xff
	brc	0x40	; arg = 0x13f
	hlt	042

	lw	r0, 0xffff
	md	0xff00
	brc	0	; arg = 0xff00
	hlt	043

	lwt	r0, 0
	md	0xffff
	brc	1	; arg = 0
	ujs	1
	hlt	044

	hlt	077
