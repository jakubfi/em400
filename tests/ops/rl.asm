
	lwt	r5, 10
	lwt	r6, 20
	lwt	r7, 30

	rl data

	hlt 077

	.org	20
data:

; XPCT int(rz[6]) : 0
; XPCT int(sr) : 0

; XPCT int([20]) : 10
; XPCT int([21]) : 20
; XPCT int([22]) : 30
