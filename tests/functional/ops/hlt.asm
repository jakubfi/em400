; OPTS -c configs/minimal-clock.cfg

	lw	r1, 0b0000100000000000
	rw	r1, 20
	lw	r1, fin
	rw	r1, 69
	im	20
loop:	hlt	0
	ujs	loop

fin:	hlt	077

; XPCT rz[6] : 0

; XPCT sr : 0
; XPCT rz[5] : 0
; XPCT ir&0x3f : 0o77
