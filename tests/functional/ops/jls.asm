
	lw	r0, ~?L
	jls	fail
	lw	r0, ?L
	jls	fin
fail:	hlt	040
fin:	hlt	077

