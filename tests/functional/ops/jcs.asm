
	lw	r0, ~?C
	jcs	fail
	lw	r0, ?C
	jcs	fin
fail:	hlt	040
fin:	hlt	077

