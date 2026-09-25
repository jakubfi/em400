
	lw	r0, ~?V
	jvs	fail
	lw	r0, ?V
	jvs	fin
fail:	hlt	040
fin:	hlt	077


; XPCT r0 : 0b0000000000000000
