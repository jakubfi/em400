; PRECMD clock on

	.include cpu.inc

	lw	r1, stack
	rw	r1, STACKP
	lw	r1, fin
	rw	r1, INTV_TIMER
	im	mask
loop:	hlt
	ujs	loop

fin:	hlt	077
mask:	.word	IMASK_GROUP_H
stack:

; XPCT rz[6] : 0

; XPCT sr : 0
; XPCT ir : 0xec3f
