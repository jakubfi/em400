
	lw	r0, ~?Y
	jys	fail
	lw	r0, ?Y
	jys	next
fail:	hlt	040
next:	lw	r0, -1
	jys	fin
	hlt	040
fin:	hlt	077

; XPCT r0 : 0xffff
