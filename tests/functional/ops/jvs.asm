
	lw	r0, ~?V
	jvs	fail
	lw	r0, ?V
	jvs	next
fail:	hlt	040
next:	lw	r0, -1
	jvs	fin
	hlt	040
fin:	hlt	077

; XPCT r0 : 0xdfff
