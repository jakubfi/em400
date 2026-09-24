; MD argument is pre-modified, then B-modified, then D-modified

	lwt	r1, 2
	lwt	r2, 3
	lw	r3, tab

	md	r1+r2
	md	r1+r2
	lwt	r4, 1

	md	1
	md	[tab]
	lwt	r5, 1

	md	2
	md	[r3]
	lwt	r6, 1

	md	1
	md	[r3+r1]
	lwt	r7, 1

	hlt	077

tab:	.word	10, 20, 30, 40

; XPCT ir : 0xec3f
; XPCT rz[6] : 0

; XPCT r4 : 11
; XPCT r5 : 21
; XPCT r6 : 31
; XPCT r7 : 41
