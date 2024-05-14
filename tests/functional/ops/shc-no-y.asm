; SHC does not touch CPU flags

	lwt	r0, 0
	lw	r1, 0xffff
	shc	r1, 1

	hlt	077

; XPCT rz[6] : 0
; XPCT sr : 0

; XPCT r0 : 0
