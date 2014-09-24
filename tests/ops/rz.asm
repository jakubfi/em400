
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

; XPCT int(rz[6]) : 0
; XPCT int(sr) : 0

; XPCT int([106]) : -1
; XPCT int([107]) : -1
; XPCT int([108]) : -1
; XPCT int([109]) : -1
; XPCT int([110]) : 0
; XPCT int([112]) : -1
; XPCT int([113]) : -1
; XPCT int([114]) : -1
; XPCT int([115]) : -1
; XPCT int([116]) : -1
