
	lw	r0, ~?L
	jl	fail
	lw	r0, ?L
	jl	fin
fail:	hlt	040
fin:	hlt	077

; XPCT rz[6] : 0
