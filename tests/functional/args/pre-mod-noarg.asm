; instruction with no arg resets modification

	md	5
	ric	r1
	lwt	r2, 1

	lwt	r3, 1
	md	5
	shc	r3, 1
	lwt	r4, 1

	hlt	077

; XPCT ir : 0xec3f
; XPCT rz[6] : 0
; XPCT mc : 0

; XPCT r2 : 1
; XPCT r3 : 0x8000
; XPCT r4 : 1
