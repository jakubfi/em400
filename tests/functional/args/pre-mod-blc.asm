; pre-modification affects BLC byte argument
; premod is added to the byte arg
; 16-bit result's right byte is tested against r0's left byte
; any higher bit always skips

	lw	r0, 0x0100
	md	1
	blc	0x0100	; arg = 2
	hlt	040

	lw	r0, 0x7f00
	md	0x3f
	blc	0x4000	; arg = 0x7f
	ujs	1
	hlt	041

	lw	r0, 0xffff
	md	0xff
	blc	0x4000	; arg = 0x13f
	hlt	042

	lw	r0, 0xffff
	md	0xff00
	blc	0	; arg = 0xff00
	hlt	043

	hlt	077
