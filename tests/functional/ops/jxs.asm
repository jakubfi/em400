
	lw	r0, ~?X
	jxs	fail
	lw	r0, ?X
	jxs	fin
fail:	hlt	040
fin:	hlt	077

; XPCT rz[6] : 0
; XPCT sr : 0
