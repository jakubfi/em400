
	lw	r0, ~?C
	jcs	fail
	lw	r0, ?C
	jcs	next
fail:	hlt	040
next:	lw	r0, -1
	jcs	fin
	hlt	040
fin:	hlt	077

; XPCT r0 : 0xffff
