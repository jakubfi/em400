
	lw	r0, ~?X
	jxs	fail
	lw	r0, ?X
	jxs	next
fail:	hlt	040
next:	lw	r0, -1
	jxs	fin
	hlt	040
fin:	hlt	077

; XPCT r0 : 0xffff
