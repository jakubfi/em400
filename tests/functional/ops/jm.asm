
	lw	r0, ~?M
	jm	fail
	lw	r0, ?M
	jm	fin
fail:	hlt	040
fin:	hlt	077

