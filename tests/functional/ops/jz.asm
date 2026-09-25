
	lw	r0, ~?Z & 0xffff
	jz	fail
	lw	r0, ?Z
	jz	fin
fail:	hlt	040
fin:	hlt	077

