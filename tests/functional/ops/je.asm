
	lw	r0, ~?E
	je	fail
	lw	r0, ?E
	je	fin
fail:	hlt	040
fin:	hlt	077

; XPCT rz[6] : 0
