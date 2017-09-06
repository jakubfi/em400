
	.cpu	mera400

	lwt	r1, 0
	lwt	r2, 0
	lwt	r3, 0
	lwt	r4, 0
	lwt	r5, 0
	lwt	r6, 0
	lwt	r7, 0
	ll	data

	hlt	077

data:	.word	0xfafa, 0xbaba, 0xdada

; XPCT r1 : 0
; XPCT r2 : 0
; XPCT r3 : 0
; XPCT r4 : 0
; XPCT r5 : 0xfafa
; XPCT r6 : 0xbaba
; XPCT r7 : 0xdada
