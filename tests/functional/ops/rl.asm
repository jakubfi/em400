
	lwt	r5, 10
	lwt	r6, 20
	lwt	r7, 30

	rl	data

	hlt	077

	.org	20
data:

; XPCT rz[6] : 0
; XPCT sr : 0

; XPCT [20] : 10
; XPCT [21] : 20
; XPCT [22] : 30
