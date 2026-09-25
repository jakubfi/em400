
	lw	r0, ~?E
	jes	fail
	lw	r0, ?E
	jes	next
fail:	hlt	040
next:	lw	r0, -1
	jes	fin
	hlt	040
fin:	hlt	077

; XPCT r0 : 0xffff
