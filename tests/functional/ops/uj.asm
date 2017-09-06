
	lw	r1, label
	uj	r1
	hlt	040
label:	hlt	077
	hlt	040
	hlt	040

; XPCT rz[6] : 0
; XPCT sr : 0

; XPCT ir : 0xec3f
