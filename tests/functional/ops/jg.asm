
	lw	r0, ~?G
	jg	fail
	lw	r0, ?G
	jg	fin
fail:	hlt	040
fin:	hlt	077

; XPCT rz[6] : 0
