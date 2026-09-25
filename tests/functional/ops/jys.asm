
	lw	r0, ~?Y
	jys	fail
	lw	r0, ?Y
	jys	fin
fail:	hlt	040
fin:	hlt	077

; XPCT rz[6] : 0
