; pre-modification affects byte argument

	lw	r0, 1	; r0 = 1
	md	1
	brc	1	; branch if r0 != 1+1
	hlt	040

	lw	r0, 0x0100
	md	0x0100
	blc	0x0100	; branch if r0 != 0x0100 + 0x0100
	hlt	041

	hlt	077

; XPCT ir : 0xec3f
