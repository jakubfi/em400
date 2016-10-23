
	lwt	r0, -10
	lwt	r1, -11
	lwt	r2, -12
	lwt	r3, -13
	lwt	r4, -14
	lwt	r5, -15
	lwt	r6, -16
	lwt	r7, -17

	rw	r0, 120
	rw	r1, 121
	rw	r2, 122
	rw	r3, 123
	rw	r4, 124
	rw	r5, 125
	rw	r6, 126
	rw	r7, 127

	hlt	077

; XPCT rz[6] : 0
; XPCT sr : 0

; XPCT [120] : -10
; XPCT [121] : -11
; XPCT [122] : -12
; XPCT [123] : -13
; XPCT [124] : -14
; XPCT [125] : -15
; XPCT [126] : -16
; XPCT [127] : -17
