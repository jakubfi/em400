
	lw	r0, ~?L
	jls	fail
	lw	r0, ?L
	jls	next
fail:	hlt	040
next:	lw	r0, -1
	jls	fin
	hlt	040
fin:	hlt	077

; XPCT r0 : 0xffff
