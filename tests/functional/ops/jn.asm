
	lw	r0, ?E
	jn	fail
	lw	r0, ~?E
	jn	fin
fail:	hlt	040
fin:	hlt	077

; XPCT rz[6] : 0
; XPCT sr : 0
