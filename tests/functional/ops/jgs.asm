
	lw	r0, ~?G
	jgs	fail
	lw	r0, ?G
	jgs	next
fail:	hlt	040
next:	lw	r0, -1
	jgs	fin
	hlt	040
fin:	hlt	077

; XPCT r0 : 0xffff
