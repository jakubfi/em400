; legal instruction resets MOD

	md	1
	md	1
	md	1
	lwt	r1, 0
	lwt	r2, 0

	hlt	077

; XPCT sr : 0
; XPCT rz[6] : 0

; XPCT mc : 0
; XPCT r1 : 3
; XPCT r2 : 0
