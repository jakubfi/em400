
	lw	r0, ~?V
	jvs	fail
	lw	r0, ?V
	jvs	fin
fail:	hlt	040
fin:	hlt	077

; XPCT rz[6] : 0
; XPCT sr : 0

; XPCT ir : 0xec3f
; XPCT r0 : 0b0000000000000000
