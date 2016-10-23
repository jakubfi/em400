
	lwt	r1, -1
	rw	r1, 106
	rw	r1, 107
	rw	r1, 108
	rw	r1, 109
	rw	r1, 110
	rw	r1, 111
	rw	r1, 112
	rw	r1, 113
	rw	r1, 114
	rw	r1, 115
	rw	r1, 116

	rz	110

	hlt	077

; XPCT rz[6] : 0
; XPCT sr : 0

; XPCT [106] : -1
; XPCT [107] : -1
; XPCT [108] : -1
; XPCT [109] : -1
; XPCT [110] : 0
; XPCT [112] : -1
; XPCT [113] : -1
; XPCT [114] : -1
; XPCT [115] : -1
; XPCT [116] : -1
