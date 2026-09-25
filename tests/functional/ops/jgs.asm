
	lw	r0, ~?G
	jgs	fail
	lw	r0, ?G
	jgs	fin
fail:	hlt	040
fin:	hlt	077

; XPCT rz[6] : 0
