; ineffective instructions reset pre-modification

	lwt	r0, 0

	md	5
	jz	fail
	lwt	r1, 1

	md	1
	jz	fail
	md	1
	md	1
	md	1		; would be the illegal 4th MD if jz did'nt reset pre-mod
	lwt	r2, 0

	lwt	r3, -1
	md	3
	trb	r3, -2
	md	7		; skipped
	lwt	r4, 1

	hlt	077
fail:	hlt	040

; XPCT ir : 0xec3f
; XPCT rz[6] : 0
; XPCT mc : 0

; XPCT r1 : 1
; XPCT r2 : 3
; XPCT r3 : 0
; XPCT r4 : 1
