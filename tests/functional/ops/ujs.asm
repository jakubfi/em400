
	; ujs does not clear any flags

	lw	r0, -1
	ujs	0
	ujs	fw	; farthest forward jump
	hlt	040

bw:	ujs	fin
	.res	61

fw:	ujs	bw	; farthest backward jump
	hlt	040

fin:	hlt	077

; XPCT r0 : 0xffff
