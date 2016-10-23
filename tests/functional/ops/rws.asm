
	lwt	r0, -30
	lwt	r1, -31
	lwt	r2, -32
	lwt	r3, -33
	lwt	r4, -34
	lwt	r5, -35
	lwt	r6, -36
	lwt	r7, -37

	rws r0, data1
	rws r1, data1+1
	rws r2, data1+2
	rws r3, data1+3
	rws r4, data2
	rws r5, data2+1
	rws r6, data2+2
	rws r7, data2+3

	hlt 077

	.org	20
data1:	.res	5
	.org	30
data2:	.res	5

; XPCT rz[6] : 0
; XPCT sr : 0

; XPCT [20] : -30
; XPCT [21] : -31
; XPCT [22] : -32
; XPCT [23] : -33
; XPCT [30] : -34
; XPCT [31] : -35
; XPCT [32] : -36
; XPCT [33] : -37
